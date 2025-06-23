import logging
from fastapi import FastAPI, Request, status
from fastapi.responses import JSONResponse
from fastapi.exceptions import RequestValidationError
from contextlib import asynccontextmanager

from app.config_forge import settings
from app.storage_manager import init_db, close_db_connection
from app.system_info_forge import router as system_router
from app.forge_manager import router as forge_router
from app.models_forge import HTTPErrorDetail

logging.basicConfig(level=settings.LOG_LEVEL.upper())
logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info(f"Starting {settings.RELIC_FORGE_SERVICE_NAME} v{settings.RELIC_FORGE_VERSION}")
    logger.info(f"Uvicorn internal host: {settings.FORGE_API_HOST}")
    logger.info(f"Uvicorn internal port: {settings.FORGE_INTERNAL_PORT}")
    logger.info(f"Database URL: {settings.FORGE_DATABASE_URL}")
    logger.info(f"Forged Relics Directory: {settings.FORGED_RELICS_DIR}")
    await init_db()
    logger.info("Database initialized.")
    yield
    await close_db_connection()
    logger.info("Database connection closed.")
    logger.info(f"Shutting down {settings.RELIC_FORGE_SERVICE_NAME}.")

app = FastAPI(
    title=settings.RELIC_FORGE_SERVICE_NAME,
    version=settings.RELIC_FORGE_VERSION,
    lifespan=lifespan,
    description="API for forging and managing modular, containerized software relics."
)

@app.exception_handler(RequestValidationError)
async def validation_exception_handler(request: Request, exc: RequestValidationError):
    return JSONResponse(
        status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
        content=HTTPErrorDetail(message="Input validation failed", detail=exc.errors()).model_dump(),
    )

@app.exception_handler(Exception)
async def generic_exception_handler(request: Request, exc: Exception):
    logger.error(f"Unhandled exception: {exc}", exc_info=True)
    return JSONResponse(
        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
        content=HTTPErrorDetail(message="An unexpected error occurred.", detail=str(exc)).model_dump(),
    )

app.include_router(system_router, prefix="/system", tags=["System"])
app.include_router(forge_router, prefix="/forge", tags=["Forge"])

@app.get("/", tags=["Root"], summary="Root endpoint providing basic API information")
async def read_root():
    return {
        "message": f"Welcome to {settings.RELIC_FORGE_SERVICE_NAME}",
        "version": settings.RELIC_FORGE_VERSION,
        "docs_url": "/docs",
        "redoc_url": "/redoc"
    }

if __name__ == "__main__":
    import uvicorn
    # When running directly, use FORGE_INTERNAL_PORT for uvicorn
    uvicorn.run("app.main:app", host=settings.FORGE_API_HOST, port=settings.FORGE_INTERNAL_PORT, log_level=settings.LOG_LEVEL.lower(), reload=True)
