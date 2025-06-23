import logging
from fastapi import APIRouter, HTTPException

from app.models_gateway import ChatCompletionRequest, ChatCompletionResponse, ErrorDetailResponse
from app.services_gateway.router_logic import route_chat_completion

logger = logging.getLogger(__name__)
router = APIRouter()

@router.post("/chat/completions", 
            response_model=ChatCompletionResponse,
            summary="Unified endpoint for chat completions across different AI providers.",
            responses={
                400: {"model": ErrorDetailResponse, "description": "Invalid request (e.g., unsupported provider)"},
                500: {"model": ErrorDetailResponse, "description": "Internal server error or provider error"},
                503: {"model": ErrorDetailResponse, "description": "Error communicating with the AI provider"}
            })
async def chat_completions_endpoint(request: ChatCompletionRequest):
    """
    Receives a chat completion request and routes it to the specified AI provider.

    - **provider**: Identifier for the AI provider (e.g., 'gemini', 'groq').
    - **model**: Specific model name for the provider. If None, provider's default is used.
    - **messages**: A list of chat messages, similar to OpenAI's format.
    - **max_tokens**: Maximum tokens for the generated response.
    - **temperature**: Sampling temperature.
    - **stream**: (Boolean) Whether to stream results (currently stubbed/simulated).
    """
    try:
        # The actual call to the provider is handled by the router_logic
        # which selects the appropriate connector.
        return await route_chat_completion(request)
    except HTTPException as e:
        # Re-raise HTTPExceptions that might come from router_logic or connectors
        raise e
    except ValueError as ve:
        logger.warning(f"Validation error in chat completions request: {ve}")
        raise HTTPException(status_code=400, detail=str(ve))
    except Exception as e:
        logger.error(f"Unexpected error in chat_completions_endpoint: {e}", exc_info=True)
        # This will be caught by the generic exception handler in main.py
        raise HTTPException(status_code=500, detail="An unexpected error occurred processing your request.")

# Example for a potential text generation endpoint (not fully implemented here)
# @router.post("/text/generate", response_model=TextGenerationResponse)
# async def text_generation_endpoint(request: TextGenerationRequest):
#     # Similar logic to route to appropriate text generation connectors
#     raise HTTPException(status_code=501, detail="Text generation endpoint not yet implemented.")
