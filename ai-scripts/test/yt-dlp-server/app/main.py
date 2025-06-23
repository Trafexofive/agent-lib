from fastapi import FastAPI, HTTPException, Request
from fastapi.responses import FileResponse, JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from contextlib import asynccontextmanager
import logging
import os
from uuid import uuid4

from app.config_forge import settings  # Updated import name
from app.forge_manager import ForgeManager
from app.storage_manager import StorageManager, Base  # Import Base for DB creation
from app.models_forge import RelicPlan, RelicMetadata # Assuming RelicMetadata is Pydantic here for response_model
from app.system_info_forge import SystemInfo

# Configure logging
logging.basicConfig(level=os.getenv("LOG_LEVEL", "INFO").upper())
logger = logging.getLogger(__name__)

storage_manager = StorageManager()

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info(f"Starting {settings.app_name} v{settings.app_version} in {settings.app_env} mode.")
    logger.info(f"Storage path: {settings.storage_path}")
    logger.info(f"Allowed CORS origins: {settings.allowed_origins}")
    # Ensure storage directories exist (safer to do it here than rely on Dockerfile alone)
    os.makedirs(os.path.join(settings.storage_path, "forged_relics"), exist_ok=True)
    os.makedirs(os.path.join(settings.storage_path, "forge_db"), exist_ok=True)
    logger.info("Storage directories checked/created.")
    await storage_manager.init_db()
    logger.info("Database initialized.")
    yield
    logger.info(f"Shutting down {settings.app_name}.")

app = FastAPI(
    title=settings.app_name,
    version=settings.app_version,
    lifespan=lifespan
)

# Parse CORS origins from string to list
cors_origins = [origin.strip() for origin in settings.allowed_origins.split(',') if origin.strip()]
if not cors_origins:
    logger.warning("ALLOWED_ORIGINS is empty. Defaulting to restrictive CORS.")
    cors_origins = [] # Or some sensible default like where your frontend is expected

app.add_middleware(
    CORSMiddleware,
    allow_origins=cors_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

forge_manager = ForgeManager()
system_info = SystemInfo()

# Pydantic model for response, distinct from SQLAlchemy model if necessary
class RelicMetadataResponse(RelicMetadata):
    pass # Inherits fields, can add/override later if API response differs from DB model

@app.post("/forge/relic", response_model=RelicMetadataResponse)
async def forge_relic_endpoint(plan: RelicPlan):
    try:
        relic_id = str(uuid4())
        # ForgeManager.forge_relic now returns the name of the package (e.g., relic_id.tar.gz)
        # and the full path to the directory containing the artifacts before packaging.
        package_filename, _ = await forge_manager.forge_relic(relic_id, plan)
        
        # Construct metadata for storage
        db_metadata = storage_manager.create_db_metadata_object(
            relic_id=relic_id,
            relic_name=plan.relic_name,
            package_filename=package_filename, # This is now just the filename
            download_url=f"/forge/relics/{relic_id}/download",
            status="forged",
            timestamp_str=system_info.get_current_timestamp(), # Ensure this method exists and returns str
            version=plan.version
        )
        await storage_manager.store_metadata(db_metadata)
        
        # Return Pydantic response model
        return RelicMetadataResponse(**db_metadata.to_dict()) # Convert SQLAlchemy model to dict for Pydantic
    except Exception as e:
        logger.error(f"Forge operation failed: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Forge failed: {str(e)}")

@app.get("/forge/relics", response_model=list[RelicMetadataResponse])
async def list_relics_endpoint():
    try:
        db_relics = await storage_manager.list_relics()
        return [RelicMetadataResponse(**relic.to_dict()) for relic in db_relics]
    except Exception as e:
        logger.error(f"List relics operation failed: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"List failed: {str(e)}")

@app.get("/forge/relics/{relic_id}/download", response_class=FileResponse)
async def download_relic_endpoint(relic_id: str):
    try:
        metadata = await storage_manager.get_relic_metadata_by_id(relic_id)
        if not metadata:
            raise HTTPException(status_code=404, detail="Relic metadata not found")
        
        # package_filename is stored in metadata
        package_path = os.path.join(settings.storage_path, "forged_relics", metadata.package_filename)

        if not await aiofiles.os.path.exists(package_path):
            logger.error(f"Relic package file not found at {package_path} for relic_id {relic_id}")
            raise HTTPException(status_code=404, detail="Relic package file not found on server")
        return FileResponse(package_path, filename=metadata.package_filename)
    except HTTPException: # Re-raise HTTPExceptions directly
        raise
    except Exception as e:
        logger.error(f"Download operation for relic {relic_id} failed: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Download failed: {str(e)}")

@app.delete("/forge/relics/{relic_id}", status_code=200) # 204 No Content is also common for DELETE
async def delete_relic_endpoint(relic_id: str):
    try:
        deleted = await storage_manager.delete_relic(relic_id)
        if not deleted:
            raise HTTPException(status_code=404, detail="Relic not found or already deleted")
        return {"status": "deleted", "relic_id": relic_id}
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Delete operation for relic {relic_id} failed: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Delete failed: {str(e)}")

@app.get("/system/health")
async def health_check_endpoint():
    # This endpoint typically should not raise exceptions itself for basic checks
    return system_info.get_health_status()

@app.get("/system/capabilities")
async def get_capabilities_endpoint():
    return system_info.get_capabilities()

# Custom error handler for HTTPException to ensure JSON response
@app.exception_handler(HTTPException)
async def custom_http_exception_handler(request: Request, exc: HTTPException):
    return JSONResponse(
        status_code=exc.status_code,
        content={"detail": exc.detail} # Changed 'error' to 'detail' to match FastAPI default
    )

# Generic error handler for unhandled exceptions
@app.exception_handler(Exception)
async def generic_exception_handler(request: Request, exc: Exception):
    logger.error(f"Unhandled exception: {exc}", exc_info=True)
    return JSONResponse(
        status_code=500,
        content={"detail": "An unexpected internal server error occurred."}
    )

if __name__ == "__main__":
    import uvicorn
    # This block is for local development running `python app/main.py`
    # Docker CMD uses a direct uvicorn call.
    uvicorn.run(app, host="0.0.0.0", port=settings.app_port_internal, reload=True)
