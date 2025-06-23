import logging
from fastapi import HTTPException

from app.models_gateway import ChatCompletionRequest, ChatCompletionResponse
from app.services_gateway.gemini_connector import GeminiConnector
from app.services_gateway.groq_connector import GroqConnector
# Import other connectors here as they are added

logger = logging.getLogger(__name__)

# Initialize connectors (could be done more dynamically, e.g., based on config)
gemini_connector = GeminiConnector()
groq_connector = GroqConnector()
# other_connector = OtherConnector()

PROVIDER_MAP = {
    "gemini": gemini_connector,
    "gemini-free": gemini_connector, # Alias
    "groq": groq_connector,
    "groq-cloud": groq_connector, # Alias
    # "other_provider": other_connector,
}

async def route_chat_completion(
    request: ChatCompletionRequest
) -> ChatCompletionResponse:
    provider_id_lower = request.provider.lower()
    connector = PROVIDER_MAP.get(provider_id_lower)

    if not connector:
        logger.error(f"Unsupported or unknown provider: {request.provider}")
        valid_providers = list(PROVIDER_MAP.keys())
        raise HTTPException(
            status_code=400, 
            detail=f"Unsupported provider: '{request.provider}'. Valid providers are: {valid_providers}"
        )

    logger.info(f"Routing chat completion request to provider: {connector.provider_id} for model: {request.model or connector.default_model}")
    try:
        return await connector.create_chat_completion(request)
    except HTTPException as e: # Re-raise HTTPExceptions from connectors
        raise e
    except Exception as e:
        logger.error(f"Error during routing or from connector '{connector.provider_id}': {e}", exc_info=True)
        raise HTTPException(status_code=503, detail=f"Error communicating with provider '{connector.provider_id}': {str(e)}")
