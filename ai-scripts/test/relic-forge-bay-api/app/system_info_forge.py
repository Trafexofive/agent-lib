import logging
import datetime
from fastapi import APIRouter, HTTPException, FastAPI
from typing import List, Dict, Any

from app.models_forge import SystemHealth, SystemCapabilities, CapabilityDetail, FullRelicPlanInput, RelicForgedResponse, RelicMetadata
from app.config_forge import settings

logger = logging.getLogger(__name__)
router = APIRouter()

@router.get("/health", 
            response_model=SystemHealth,
            summary="Get system health status",
            tags=["System"])
async def get_system_health():
    return SystemHealth(
        service_name=settings.RELIC_FORGE_SERVICE_NAME,
        version=settings.RELIC_FORGE_VERSION
    )

@router.get("/capabilities", 
            response_model=SystemCapabilities,
            summary="Get system capabilities information",
            tags=["System"])
async def get_system_capabilities():
    capabilities_list = [
        CapabilityDetail(
            endpoint="/forge/relic", 
            method="POST", 
            description="Forge a new relic from a JSON plan.",
            request_body_schema=FullRelicPlanInput.model_json_schema(ref_template="#/components/schemas/{model}"),
            response_schema=RelicForgedResponse.model_json_schema(ref_template="#/components/schemas/{model}")
        ),
        CapabilityDetail(
            endpoint="/forge/relics", 
            method="GET", 
            description="List all forged relics.",
            response_schema={"type": "array", "items": RelicMetadata.model_json_schema(ref_template="#/components/schemas/{model}")}
        ),
        CapabilityDetail(
            endpoint="/forge/relics/{relic_id}", 
            method="GET", 
            description="Get metadata for a specific relic.",
            response_schema=RelicMetadata.model_json_schema(ref_template="#/components/schemas/{model}")
        ),
        CapabilityDetail(
            endpoint="/forge/relics/{relic_id}/download", 
            method="GET", 
            description="Download a forged relic package (.tar.gz).",
            response_schema={"type": "string", "format": "binary", "description": "A .tar.gz file"}
        ),
        CapabilityDetail(
            endpoint="/forge/relics/{relic_id}", 
            method="DELETE", 
            description="Delete a forged relic (metadata and package).",
            response_schema={"description": "204 No Content"} 
        ),
        CapabilityDetail(
            endpoint="/system/health", 
            method="GET", 
            description="Get system health status.",
            response_schema=SystemHealth.model_json_schema(ref_template="#/components/schemas/{model}")
        ),
        CapabilityDetail(
            endpoint="/system/capabilities", 
            method="GET", 
            description="Get system capabilities information.",
            response_schema=SystemCapabilities.model_json_schema(ref_template="#/components/schemas/{model}")
        )
    ]
    
    return SystemCapabilities(
        service_name=settings.RELIC_FORGE_SERVICE_NAME,
        version=settings.RELIC_FORGE_VERSION,
        capabilities=capabilities_list
    )
