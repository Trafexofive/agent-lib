import os
import logging
from logging.config import dictConfig
from pathlib import Path
import asyncio

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from python_json_logger import jsonlogger # Import the class directly

from gap_core.core.loader import ComponentLoader
from gap_core.core.engine import OrchestrationEngine

# Correctly configure logging by passing the class object, not a string
dictConfig({
    'version': 1,
    'disable_existing_loggers': False,
    'formatters': {'json': {'()': jsonlogger.JsonFormatter, 'format': '%(asctime)s %(levelname)s %(name)s %(message)s'}},
    'handlers': {
        'json': {'class': 'logging.handlers.RotatingFileHandler', 'formatter': 'json', 'filename': '/app/logs/gap_core.log', 'maxBytes': 10485760, 'backupCount': 5},
        'console': {'class': 'logging.StreamHandler', 'formatter': 'json'},
    },
    'loggers': {'': {'handlers': ['json', 'console'], 'level': os.getenv('LOG_LEVEL', 'INFO').upper()}}
})
logger = logging.getLogger(__name__)

app = FastAPI(title="GAP API", version="1.1.1")

# --- CORS Configuration ---
origins = os.getenv("CORS_ALLOWED_ORIGINS", "").split()
app.add_middleware(CORSMiddleware, allow_origins=origins if origins else ["*"], allow_credentials=True, allow_methods=["*"], allow_headers=["*"])

# --- Core Component Loading & Engine Startup ---
@app.on_event("startup")
async def startup_event():
    logger.info("GAP Backend starting up...")
    WORKSPACE_DIR = Path("/app/workspace")
    try:
        loader = ComponentLoader(WORKSPACE_DIR)
        loader.load_all()
        app.state.component_loader = loader
        logger.info("ComponentLoader initialized and loaded successfully.")

        engine = OrchestrationEngine(loader)
        app.state.engine = engine
        asyncio.create_task(engine.run())
        logger.info("OrchestrationEngine started in background.")

    except Exception as e:
        logger.error(f"CRITICAL: Failed during startup sequence: {e}", exc_info=True)
        app.state.component_loader = None
        app.state.engine = None

def get_loader():
    if not hasattr(app.state, 'component_loader') or not app.state.component_loader:
        raise HTTPException(status_code=503, detail="ComponentLoader unavailable due to startup error.")
    return app.state.component_loader

# --- API Endpoints ---
@app.get("/")
def read_root(): return {"message": "Welcome to GAP v1.1.1. Engine is running."}

@app.get("/context")
def get_context(): return {"relic_name": "generic-automation-platform", "version": "1.1.1"}

@app.get("/api/v1/components/reload")
def reload_components():
    loader = get_loader()
    loader.load_all()
    return {"status": "success", "loaded": {"agents": len(loader.agents), "workflows": len(loader.workflows)}}
