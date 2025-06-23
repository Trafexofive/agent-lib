from .base_client import BaseLLMClient
from .gemini_client import GeminiClient
from .groq_client import GroqClient

class LLMClientFactory:
    _clients = {}
    @staticmethod
    def get_client(model_name: str) -> BaseLLMClient:
        provider, _ = model_name.lower().split('/', 1)
        if provider not in LLMClientFactory._clients:
            if provider == 'gemini': LLMClientFactory._clients[provider] = GeminiClient()
            elif provider == 'groq': LLMClientFactory._clients[provider] = GroqClient()
            else: raise ValueError(f"Unsupported LLM provider: '{provider}'")
        return LLMClientFactory._clients[provider]
