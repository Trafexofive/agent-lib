import logging
import os
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware

from app.config_registry import settings
from app.storage_registry import init_db, close_db_connection
from app.api_routes_registry import router as registry_router
from app.system_info_registry import router as system_router
from app.models_registry import ErrorDetailResponse

# Configure logging based on LOG_LEVEL from settings
log_level = getattr(logging, settings.LOG_LEVEL.upper(), logging.INFO)
logging.basicConfig(level=log_level, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app_instance: FastAPI):
    logger.info(f"Starting {settings.APP_NAME} v{settings.APP_VERSION}...")
    logger.info(f"Allowed CORS origins: {settings.CORS_ALLOWED_ORIGINS_LIST}")
    logger.info(f"Uploaded assets directory (container): {settings.UPLOADED_ASSETS_DIR_CONTAINER}")
    logger.info(f"Database URL path (container): {settings.REGISTRY_DB_URL_SQLITE_PATH}")
    
    # Ensure storage directories exist (Docker volumes should handle this, but good for dev)
    os.makedirs(settings.UPLOADED_ASSETS_DIR_CONTAINER, exist_ok=True)
    os.makedirs(os.path.dirname(settings.REGISTRY_DB_URL_SQLITE_PATH), exist_ok=True)
    
    await init_db()
    logger.info("Database initialized.")
    yield
    await close_db_connection()
    logger.info("Database connection closed.")
    logger.info(f"Shutting down {settings.APP_NAME}.")

app = FastAPI(
    title=settings.APP_NAME,
    version=settings.APP_VERSION,
    lifespan=lifespan
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.CORS_ALLOWED_ORIGINS_LIST,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(registry_router, prefix="/registry", tags=["Registry"])
app.include_router(system_router, prefix="/system", tags=["System"])

@app.get("/", tags=["Root"])
async def read_root():
    return {
        "message": f"Welcome to {settings.APP_NAME} v{settings.APP_VERSION}",
        "docs_url": "/docs"
    }

# Custom error handlers
@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    return JSONResponse(
        status_code=exc.status_code,
        content=ErrorDetailResponse(detail=exc.detail).model_dump(),
    )

@app.exception_handler(Exception)
async def generic_exception_handler(request: Request, exc: Exception):
    logger.error(f"Unhandled exception: {exc}", exc_info=True)
    return JSONResponse(
        status_code=500,
        content=ErrorDetailResponse(detail="An unexpected internal server error occurred.").model_dump(),
    )

if __name__ == "__main__":
    import uvicorn
    # This uvicorn call is for local development (e.g., python app/main.py)
    # The Docker CMD uses its own uvicorn call with port defined by env var APP_PORT_INTERNAL
    # For local run, using the host-exposed APP_PORT for consistency if .env is sourced.
    uvicorn_port = int(os.getenv("APP_PORT", "8009")) 
    logger.info(f"Running Uvicorn locally on port {uvicorn_port}")
    uvicorn.run(app, host="0.0.0.0", port=uvicorn_port, log_level=settings.LOG_LEVEL.lower(), reload=True)
