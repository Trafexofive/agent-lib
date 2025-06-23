import logging
import datetime
from fastapi import APIRouter

from app.config_platform import settings
from app.models_platform import HealthStatus, SystemCapabilitiesResponse, CapabilityDetail

logger = logging.getLogger(__name__)
router = APIRouter()

@router.get("/health", response_model=HealthStatus, summary="Get platform health status")
async def get_system_health():
    return HealthStatus(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        timestamp=datetime.datetime.now(datetime.timezone.utc)
    )

@router.get("/capabilities", response_model=SystemCapabilitiesResponse, summary="Get platform capabilities")
async def get_system_capabilities():
    capabilities_list = [
        CapabilityDetail(method="POST", path="/api/v1/agents/smart-create", summary="Smartly create agent (Simulated)"),
        CapabilityDetail(method="POST", path="/api/v1/agents/", summary="Manually create agent"),
        CapabilityDetail(method="GET", path="/api/v1/agents/", summary="List agents"),
        CapabilityDetail(method="GET", path="/api/v1/agents/{agent_id}", summary="Get agent by ID"),
        CapabilityDetail(method="PUT", path="/api/v1/agents/{agent_id}", summary="Update agent"),
        CapabilityDetail(method="DELETE", path="/api/v1/agents/{agent_id}", summary="Delete agent"),
        CapabilityDetail(method="POST", path="/api/v1/agents/{agent_id}/pause", summary="Pause agent"),
        CapabilityDetail(method="POST", path="/api/v1/agents/{agent_id}/resume", summary="Resume agent"),
        CapabilityDetail(method="POST", path="/api/v1/agents/import", summary="Import agents from JSON"),
        CapabilityDetail(method="GET", path="/api/v1/agents/{agent_id}/export", summary="Export agent to JSON"),
        CapabilityDetail(method="POST", path="/api/v1/agents/{agent_id}/trigger-run", summary="Manually trigger agent run (Simulated)"),
        CapabilityDetail(method="POST", path="/api/v1/agents/trigger-all-runs-test", summary="Trigger all agents (Simulated)"),
        CapabilityDetail(method="GET", path="/api/v1/reports/", summary="List reports"),
        CapabilityDetail(method="GET", path="/api/v1/reports/{report_id}", summary="Get report by ID"),
        CapabilityDetail(method="PUT", path="/api/v1/reports/{report_id}", summary="Update report"),
        CapabilityDetail(method="DELETE", path="/api/v1/reports/{report_id}", summary="Delete report"),
        CapabilityDetail(method="GET", path="/system/health", summary="System health"),
        CapabilityDetail(method="GET", path="/system/capabilities", summary="System capabilities")
    ]
    return SystemCapabilitiesResponse(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        description="Platform for autonomous reporter agents. Core CRUD for agents/reports implemented. Info gathering & smart features are simulated.",
        capabilities=capabilities_list
    )
