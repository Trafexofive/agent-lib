from aetherium_core.integrations.llm.base_client import BaseLLMClient
from aetherium_core.integrations.llm.gemini_client import GeminiClient
from aetherium_core.integrations.llm.groq_client import GroqClient

class LLMClientFactory:
    _clients = {}
    @staticmethod
    def get_client(model_name: str) -> BaseLLMClient:
        if not isinstance(model_name, str) or '/' not in model_name:
            raise ValueError(f"Invalid model name format: '{model_name}'. Expected 'provider/model'.")
        provider, _ = model_name.lower().split('/', 1)
        if provider not in LLMClientFactory._clients:
            if provider == 'gemini': LLMClientFactory._clients[provider] = GeminiClient()
            elif provider == 'groq': LLMClientFactory._clients[provider] = GroqClient()
            else: raise ValueError(f"Unsupported LLM provider: '{provider}'")
        return LLMClientFactory._clients[provider]
