from abc import ABC, abstractmethod
from typing import List, Dict

class BaseLLMClient(ABC):
    @abstractmethod
    def generate_response(self, system_prompt: str, messages: List[Dict[str, str]], model: str, temperature: float) -> str: pass
