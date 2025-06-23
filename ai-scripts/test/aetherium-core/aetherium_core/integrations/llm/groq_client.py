import os, logging
from groq import AsyncGroq
from .base_client import BaseLLMClient
from typing import List, Dict, Any

logger = logging.getLogger(__name__)

class GroqClient(BaseLLMClient):
    def __init__(self):
        api_key = os.getenv("GROQ_API_KEY")
        self.configured = bool(api_key and 'YOUR_GROQ_API_KEY_HERE' not in api_key)
        if self.configured: self.async_client = AsyncGroq(api_key=api_key)
        else: logger.warning("GroqClient not configured.")

    async def generate_response(self, system_prompt: str, messages: List[Dict[str, str]], model: str, temperature: float = 0.7) -> str:
        if not self.configured: raise RuntimeError("GroqClient not configured.")
        model_name = model.split('/', 1)[1]
        groq_messages = [{"role": "system", "content": system_prompt}] + messages
        completion = await self.async_client.chat.completions.create(messages=groq_messages, model=model_name, temperature=temperature)
        return completion.choices[0].message.content or "(Model refused to respond)"
