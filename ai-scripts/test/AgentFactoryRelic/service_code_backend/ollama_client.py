import os
import httpx
from dotenv import load_dotenv

load_dotenv()

class OllamaClient:
    def __init__(self):
        self.base_url = os.getenv("OLLAMA_API_BASE_URL")
        if not self.base_url:
            raise ValueError("OLLAMA_API_BASE_URL environment variable not set.")
        self.api_endpoint = f"{self.base_url}/api/generate"

    async def generate(self, model: str, prompt: str, temperature: float = None, stop: list = None) -> str:
        payload = {
            "model": model,
            "prompt": prompt,
            "stream": False,
            "format": "json"
        }
        if temperature is not None:
            payload['options'] = payload.get('options', {})
            payload['options']['temperature'] = temperature
        if stop is not None:
            payload['options'] = payload.get('options', {})
            payload['options']['stop'] = stop

        async with httpx.AsyncClient(timeout=120.0) as client:
            try:
                response = await client.post(self.api_endpoint, json=payload)
                response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)
                return response.json().get('response', '')
            except httpx.RequestError as e:
                print(f"An error occurred while requesting {e.request.url!r}.")
                return f'{{"error": "Failed to connect to Ollama at {self.base_url}"}}'
            except httpx.HTTPStatusError as e:
                print(f"Error response {e.response.status_code} while requesting {e.request.url!r}.")
                return f'{{"error": "Received status {e.response.status_code} from Ollama", "details": "{e.response.text}"}}'
