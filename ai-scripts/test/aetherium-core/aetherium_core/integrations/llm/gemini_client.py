import os, logging, google.generativeai as genai
from .base_client import BaseLLMClient
from typing import List, Dict, Any

logger = logging.getLogger(__name__)

class GeminiClient(BaseLLMClient):
    def __init__(self):
        api_key = os.getenv("GOOGLE_GEMINI_API_KEY")
        self.configured = bool(api_key and 'YOUR_GEMINI_API_KEY_HERE' not in api_key)
        if self.configured:
            genai.configure(api_key=api_key)
        else: logger.warning("GeminiClient not configured.")

    async def generate_response(self, system_prompt: str, messages: List[Dict[str, str]], model: str, temperature: float = 0.7) -> str:
        if not self.configured: raise RuntimeError("GeminiClient not configured.")
        model_name = model.split('/', 1)[1]
        gemini_model = genai.GenerativeModel(model_name=model_name, system_instruction=system_prompt)
        gemini_messages = [{'role': 'user' if m['role'] == 'user' else 'model', 'parts': [m['content']]} for m in messages]
        response = await gemini_model.generate_content_async(gemini_messages, generation_config=genai.types.GenerationConfig(temperature=temperature))
        return response.text if response.parts else "(Model refused to respond)"
