import logging
import datetime
from fastapi import APIRouter

from app.config_registry import settings
from app.models_registry import HealthStatus, SystemCapabilities, CapabilityDetail

logger = logging.getLogger(__name__)
router = APIRouter()

@router.get("/health", 
            response_model=HealthStatus,
            summary="Get system health status")
async def get_system_health():
    return HealthStatus(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        status="healthy",
        timestamp=datetime.datetime.now(datetime.timezone.utc)
    )

@router.get("/capabilities", 
            response_model=SystemCapabilities,
            summary="Get system capabilities information")
async def get_system_capabilities():
    capabilities_list = [
        CapabilityDetail(method="POST", path="/registry/assets", description="Upload a new asset. File via multipart/form-data, 'user_description' as form field."),
        CapabilityDetail(method="GET", path="/registry/assets", description="List all registered assets with metadata. Supports 'skip' and 'limit' query parameters."),
        CapabilityDetail(method="GET", path="/registry/assets/{asset_id}", description="Get metadata for a specific asset by its ID."),
        CapabilityDetail(method="GET", path="/registry/assets/{asset_id}/download", description="Download the specified asset file."),
        CapabilityDetail(method="DELETE", path="/registry/assets/{asset_id}", description="Delete an asset (metadata and file) by its ID."),
        CapabilityDetail(method="GET", path="/system/health", description="Get system health status."),
        CapabilityDetail(method="GET", path="/system/capabilities", description="Get this system capabilities information.")
    ]
    
    return SystemCapabilities(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        description="A bespoke registry server for assets, with (simulated) Gemini API enhancements.",
        capabilities=capabilities_list,
        gemini_integration_status="SIMULATED - Requires user implementation in app/services/gemini_service.py and valid GEMINI_API_KEY."
    )
