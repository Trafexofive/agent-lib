import logging
import os
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware

from app.config_platform import settings
from app.db_platform import init_db, close_db_connection
from app.api.agent_routes import router as agent_router
from app.api.report_routes import router as report_router
from app.api.system_routes import router as system_router
from app.models_platform import ErrorDetailResponse

log_level = getattr(logging, settings.LOG_LEVEL.upper(), logging.INFO)
logging.basicConfig(level=log_level, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app_instance: FastAPI):
    logger.info(f"Starting {settings.APP_NAME} v{settings.APP_VERSION}...")
    logger.info(f"Allowed CORS origins: {settings.CORS_ALLOWED_ORIGINS_LIST}")
    logger.info(f"Agent data directory (container): {settings.AGENT_DATA_DIR_CONTAINER}")
    logger.info(f"Database URL path (container): {settings.PLATFORM_DB_URL_SQLITE_PATH}")
    
    os.makedirs(settings.AGENT_DATA_DIR_CONTAINER, exist_ok=True)
    os.makedirs(os.path.dirname(settings.PLATFORM_DB_URL_SQLITE_PATH), exist_ok=True)
    
    await init_db()
    logger.info("Database initialized.")
    yield
    await close_db_connection()
    logger.info("Database connection closed.")
    logger.info(f"Shutting down {settings.APP_NAME}.")

app = FastAPI(
    title=settings.APP_NAME,
    version=settings.APP_VERSION,
    lifespan=lifespan,
    openapi_url="/api/v1/openapi.json",
    docs_url="/api/v1/docs",
    redoc_url="/api/v1/redoc"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.CORS_ALLOWED_ORIGINS_LIST,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(system_router, prefix="/system", tags=["System & Capabilities"])
app.include_router(agent_router, prefix="/api/v1/agents", tags=["Agent Management"])
app.include_router(report_router, prefix="/api/v1/reports", tags=["Report Management"])

@app.get("/", tags=["Root"], include_in_schema=False)
async def read_root():
    return {
        "message": f"Welcome to {settings.APP_NAME} v{settings.APP_VERSION}",
        "documentation": "/api/v1/docs"
    }

@app.exception_handler(HTTPException)
async def custom_http_exception_handler(request: Request, exc: HTTPException):
    return JSONResponse(
        status_code=exc.status_code,
        content=ErrorDetailResponse(detail=exc.detail).model_dump(),
    )

@app.exception_handler(Exception)
async def generic_exception_handler(request: Request, exc: Exception):
    logger.error(f"Unhandled exception during request to '{request.url.path}': {exc}", exc_info=True)
    return JSONResponse(
        status_code=500,
        content=ErrorDetailResponse(detail="An unexpected internal server error occurred on the platform.").model_dump(),
    )

if __name__ == "__main__":
    import uvicorn
    run_port = int(os.getenv("APP_PORT", str(settings.APP_PORT_INTERNAL)))
    logger.info(f"Running Uvicorn development server locally on http://0.0.0.0:{run_port}")
    uvicorn.run("app.main:app", host="0.0.0.0", port=run_port, log_level=settings.LOG_LEVEL.lower(), reload=True)
