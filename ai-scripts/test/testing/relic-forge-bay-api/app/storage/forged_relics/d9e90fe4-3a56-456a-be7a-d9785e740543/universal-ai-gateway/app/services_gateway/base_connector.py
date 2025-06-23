from abc import ABC, abstractmethod
import httpx
import logging
from typing import Any, Dict

from app.models_gateway import ChatCompletionRequest, ChatCompletionResponse
from app.config_gateway import settings

logger = logging.getLogger(__name__)

class BaseAIConnector(ABC):
    def __init__(self, provider_id: str, api_key: str = None, api_endpoint: str = None, default_model: str = None):
        self.provider_id = provider_id
        self.api_key = api_key
        self.api_endpoint = api_endpoint
        self.default_model = default_model
        self.timeout = settings.DEFAULT_PROVIDER_TIMEOUT

    @abstractmethod
    async def create_chat_completion(
        self, 
        request: ChatCompletionRequest
    ) -> ChatCompletionResponse:
        """Abstract method to create a chat completion using the provider's API."""
        pass

    async def _make_http_request(
        self, 
        method: str, 
        url: str, 
        headers: Dict[str, str] = None, 
        json_data: Dict[str, Any] = None,
        params: Dict[str, Any] = None
    ) -> httpx.Response:
        async with httpx.AsyncClient(timeout=self.timeout) as client:
            try:
                logger.debug(f"Making {method} request to {url} with headers: {headers} and json: {json_data}")
                response = await client.request(method, url, headers=headers, json=json_data, params=params)
                response.raise_for_status() # Raise an exception for 4xx/5xx status codes
                return response
            except httpx.HTTPStatusError as e:
                logger.error(f"HTTP error for {self.provider_id}: {e.response.status_code} - {e.response.text}")
                # Re-raise with more context or a custom exception
                raise Exception(f"{self.provider_id} API Error: {e.response.status_code} - {e.response.text}") from e
            except httpx.RequestError as e:
                logger.error(f"Request error for {self.provider_id} to {url}: {e}")
                raise Exception(f"{self.provider_id} Request Error: {str(e)}") from e
            except Exception as e:
                logger.error(f"Unexpected error during HTTP request for {self.provider_id}: {e}")
                raise Exception(f"Unexpected error contacting {self.provider_id}: {str(e)}") from e
