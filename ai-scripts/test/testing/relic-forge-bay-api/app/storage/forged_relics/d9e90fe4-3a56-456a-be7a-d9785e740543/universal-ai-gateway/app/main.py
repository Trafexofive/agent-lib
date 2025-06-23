import logging
import os
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware

from app.config_gateway import settings
from app.api.inference_routes import router as inference_router
from app.api.system_routes import router as system_router
from app.models_gateway import ErrorDetailResponse

log_level = getattr(logging, settings.LOG_LEVEL.upper(), logging.INFO)
logging.basicConfig(level=log_level, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app_instance: FastAPI):
    logger.info(f"Starting {settings.APP_NAME} v{settings.APP_VERSION}...")
    logger.info(f"Allowed CORS origins: {settings.CORS_ALLOWED_ORIGINS_LIST}")
    logger.info(f"Default provider timeout: {settings.DEFAULT_PROVIDER_TIMEOUT}s")
    # Log available (stubbed) providers based on config presence (not actual connectivity yet)
    if settings.GEMINI_API_KEY and settings.GEMINI_API_KEY != "YOUR_GEMINI_API_KEY_HERE":
        logger.info("Gemini provider (stub) configured.")
    else:
        logger.warning("Gemini provider API key not configured. Calls to Gemini will be fully simulated.")
    if settings.GROQ_API_KEY and settings.GROQ_API_KEY != "YOUR_GROQ_API_KEY_HERE":
        logger.info("Groq provider (stub) configured.")
    else:
        logger.warning("Groq provider API key not configured. Calls to Groq will be fully simulated.")
    yield
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

# API Routers
app.include_router(system_router, prefix="/system", tags=["System & Capabilities"])
app.include_router(inference_router, prefix="/api/v1/inference", tags=["AI Inference"])

@app.get("/", tags=["Root"], include_in_schema=False)
async def read_root():
    return {
        "message": f"Welcome to {settings.APP_NAME} v{settings.APP_VERSION}",
        "documentation": "/api/v1/docs"
    }

# Exception Handlers
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
        content=ErrorDetailResponse(detail="An unexpected internal server error occurred on the AI Gateway.").model_dump(),
    )

if __name__ == "__main__":
    import uvicorn
    run_port = int(os.getenv("APP_PORT", str(settings.APP_PORT_INTERNAL)))
    logger.info(f"Running Uvicorn development server locally on http://0.0.0.0:{run_port}")
    uvicorn.run("app.main:app", host="0.0.0.0", port=run_port, log_level=settings.LOG_LEVEL.lower(), reload=True)
