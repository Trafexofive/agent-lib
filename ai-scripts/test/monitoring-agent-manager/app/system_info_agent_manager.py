import logging
import datetime
from fastapi import APIRouter

from app.config_agent_manager import settings
from app.models_agent_manager import SystemHealth, SystemCapabilities

logger = logging.getLogger(__name__)
router = APIRouter()

@router.get("/health", 
            response_model=SystemHealth,
            summary="Get gateway health status")
async def get_system_health():
    return SystemHealth(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        timestamp=datetime.datetime.now(datetime.timezone.utc)
    )

@router.get("/capabilities", 
            response_model=SystemCapabilities,
            summary="Get gateway capabilities and configured provider status")
async def get_system_capabilities():
    supported_operations = [
        "Create Monitoring Agent",
        "Read Monitoring Agent(s)",
        "Update Monitoring Agent",
        "Delete Monitoring Agent"
    ]
    api_endpoints = [
        {"method": "POST", "path": "/agents", "summary": "Create a new monitoring agent."},
        {"method": "GET", "path": "/agents", "summary": "List all monitoring agents."},
        {"method": "GET", "path": "/agents/{agent_id}", "summary": "Get a specific agent by ID."},
        {"method": "PUT", "path": "/agents/{agent_id}", "summary": "Update an existing agent."},
        {"method": "DELETE", "path": "/agents/{agent_id}", "summary": "Delete an agent."},
        {"method": "GET", "path": "/system/health", "summary": "System health check."},
        {"method": "GET", "path": "/system/capabilities", "summary": "This capabilities endpoint."}
    ]
    
    return SystemCapabilities(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        description="Manages configurations for monitoring agents. Data is stored in-memory.",
        supported_operations=supported_operations,
        endpoints=api_endpoints
    )
