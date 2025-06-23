import logging
from fastapi import FastAPI, HTTPException, status, Request
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from contextlib import asynccontextmanager
from typing import List

from app.config_agent_manager import settings
from app.models_agent_manager import MonitoringAgent, MonitoringAgentCreate, MonitoringAgentUpdate, HTTPErrorDetail # Assuming HTTPErrorDetail is in models
from app.crud_agent_manager import (
    create_agent as crud_create_agent,
    get_agent as crud_get_agent,
    get_agents as crud_get_agents,
    update_agent as crud_update_agent,
    delete_agent as crud_delete_agent
)
from app.system_info_agent_manager import router as system_router

# Configure logging
log_level_val = getattr(logging, settings.LOG_LEVEL.upper(), logging.INFO)
logging.basicConfig(level=log_level_val)
logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app_instance: FastAPI):
    logger.info(f"Starting {settings.APP_NAME} v{settings.APP_VERSION}...")
    logger.info(f"CORS Allowed Origins: {settings.CORS_ALLOWED_ORIGINS_LIST}")
    # In a real app, you might initialize DB connections or other resources here
    yield
    logger.info(f"Shutting down {settings.APP_NAME}.")

app = FastAPI(
    title=settings.APP_NAME,
    version=settings.APP_VERSION,
    lifespan=lifespan,
    description="API for managing Monitoring Agent configurations."
)

# CORS Middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.CORS_ALLOWED_ORIGINS_LIST,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Custom Exception Handlers
@app.exception_handler(HTTPException)
async def custom_http_exception_handler(request: Request, exc: HTTPException):
    return JSONResponse(
        status_code=exc.status_code,
        content=HTTPErrorDetail(message=str(exc.detail)).model_dump(), # Adapt as needed
    )

@app.exception_handler(Exception)
async def generic_exception_handler(request: Request, exc: Exception):
    logger.error(f"Unhandled exception during request to '{request.url.path}': {exc}", exc_info=True)
    return JSONResponse(
        status_code=500,
        content=HTTPErrorDetail(message="An unexpected internal server error occurred.").model_dump(),
    )

# Agent CRUD Endpoints
@app.post("/agents", response_model=MonitoringAgent, status_code=status.HTTP_201_CREATED, tags=["Agents"])
async def create_new_agent(agent: MonitoringAgentCreate):
    return crud_create_agent(agent)

@app.get("/agents", response_model=List[MonitoringAgent], tags=["Agents"])
async def read_agents(skip: int = 0, limit: int = 100):
    agents = crud_get_agents(skip=skip, limit=limit)
    return agents

@app.get("/agents/{agent_id}", response_model=MonitoringAgent, tags=["Agents"])
async def read_agent(agent_id: str):
    db_agent = crud_get_agent(agent_id)
    if db_agent is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Agent not found")
    return db_agent

@app.put("/agents/{agent_id}", response_model=MonitoringAgent, tags=["Agents"])
async def update_existing_agent(agent_id: str, agent: MonitoringAgentUpdate):
    updated_agent = crud_update_agent(agent_id, agent)
    if updated_agent is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Agent not found")
    return updated_agent

@app.delete("/agents/{agent_id}", status_code=status.HTTP_204_NO_CONTENT, tags=["Agents"])
async def delete_existing_agent(agent_id: str):
    deleted_agent = crud_delete_agent(agent_id)
    if deleted_agent is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Agent not found")
    return # No content for 204

# System Router
app.include_router(system_router, prefix="/system", tags=["System"])

@app.get("/", tags=["Root"], include_in_schema=False)
async def read_root():
    return {
        "message": f"Welcome to {settings.APP_NAME} v{settings.APP_VERSION}",
        "documentation": "/docs"
    }

if __name__ == "__main__":
    import uvicorn
    # Use APP_INTERNAL_PORT for uvicorn when running directly (matches Docker CMD)
    uvicorn.run("app.main:app", host="0.0.0.0", port=settings.APP_INTERNAL_PORT, log_level=settings.LOG_LEVEL.lower(), reload=True)
