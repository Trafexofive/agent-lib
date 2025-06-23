import os, logging
from groq import Groq
from .base_client import BaseLLMClient

logger = logging.getLogger(__name__)

class GroqClient(BaseLLMClient):
    def __init__(self):
        api_key = os.getenv("GROQ_API_KEY")
        self.configured = bool(api_key and 'YOUR_GROQ_API_KEY_HERE' not in api_key)
        if self.configured: self.client = Groq(api_key=api_key)
        else: logger.warning("GroqClient not configured.")

    def generate_response(self, system_prompt, messages, model, temperature=0.7):
        if not self.configured: raise RuntimeError("GroqClient not configured.")
        model_name = model.split('/', 1)[1]
        groq_messages = [{"role": "system", "content": system_prompt}] + messages
        completion = self.client.chat.completions.create(messages=groq_messages, model=model_name, temperature=temperature)
        return completion.choices[0].message.content or "(Model refused to respond)"
