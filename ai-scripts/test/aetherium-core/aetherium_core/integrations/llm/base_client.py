from abc import ABC, abstractmethod
from typing import List, Dict, Any

class BaseLLMClient(ABC):
    @abstractmethod
    async def generate_response(self, system_prompt: str, messages: List[Dict[str, str]], model: str, temperature: float) -> str:
        """Generates a response from the large language model."""
        pass
