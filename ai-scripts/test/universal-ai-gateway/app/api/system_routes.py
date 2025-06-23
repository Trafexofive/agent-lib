import logging
import datetime
from fastapi import APIRouter

from app.config_gateway import settings
from app.models_gateway import HealthStatus, SystemCapabilitiesResponse, ProviderCapability

logger = logging.getLogger(__name__)
router = APIRouter()

@router.get("/health", 
            response_model=HealthStatus,
            summary="Get gateway health status")
async def get_system_health():
    return HealthStatus(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        timestamp=datetime.datetime.now(datetime.timezone.utc)
    )

@router.get("/capabilities", 
            response_model=SystemCapabilitiesResponse,
            summary="Get gateway capabilities and configured provider status")
async def get_system_capabilities():
    supported_providers = []
    
    # Gemini Status
    gemini_status = "key_missing"
    gemini_notes = "GEMINI_API_KEY not found in environment. Calls will be fully simulated."
    if settings.GEMINI_API_KEY and settings.GEMINI_API_KEY != "YOUR_GEMINI_API_KEY_HERE":
        gemini_status = "configured" # In reality, you might ping the API to confirm key validity
        gemini_notes = "Gemini connector is configured (calls are stubbed/simulated in this version)."
    supported_providers.append(ProviderCapability(
        provider_id="gemini", 
        status=gemini_status, 
        default_model=settings.GEMINI_DEFAULT_MODEL,
        notes=gemini_notes
    ))

    # Groq Status
    groq_status = "key_missing"
    groq_notes = "GROQ_API_KEY not found in environment. Calls will be fully simulated."
    if settings.GROQ_API_KEY and settings.GROQ_API_KEY != "YOUR_GROQ_API_KEY_HERE":
        groq_status = "configured"
        groq_notes = "Groq connector is configured (calls are stubbed/simulated in this version)."
    supported_providers.append(ProviderCapability(
        provider_id="groq", 
        status=groq_status, 
        default_model=settings.GROQ_DEFAULT_MODEL,
        notes=groq_notes
    ))
    
    # Add other providers here

    api_endpoints = [
        {"method": "POST", "path": "/api/v1/inference/chat/completions", "summary": "Unified chat completions endpoint."},
        {"method": "GET", "path": "/system/health", "summary": "System health check."},
        {"method": "GET", "path": "/system/capabilities", "summary": "This capabilities endpoint."}
    ]
    
    return SystemCapabilitiesResponse(
        app_name=settings.APP_NAME,
        version=settings.APP_VERSION,
        description="A universal AI inference gateway routing requests to various free-tier AI model providers. Connectors are stubbed and require user implementation for actual API calls.",
        supported_providers=supported_providers,
        endpoints=api_endpoints
    )
