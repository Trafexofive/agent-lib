from fastapi import FastAPI, HTTPException, Request, Depends
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
import json
import os
from pathlib import Path
import httpx
from dotenv import load_dotenv
import time # For created timestamp

load_dotenv()

from llm_providers import call_gemini_api, call_groq_api

app = FastAPI(
    title="AI Inference API Gateway",
    version="0.1.0",
    description="A unified API gateway for various free-tier AI model providers."
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

CONFIG_FILE = Path("app_config.json")

def load_app_config() -> Dict[str, Any]:
    if not CONFIG_FILE.exists():
        default_config = {
            "service_version": "0.1.0",
            "default_max_tokens": 2048,
            "models": [
                {
                    "id": "google/gemini-1.5-flash-latest",
                    "provider": "gemini",
                    "actual_model_name": "gemini-1.5-flash-latest",
                    "description": "Google Gemini Flash - Fast and versatile",
                    "free_tier_available": True,
                    "context_window": 8192
                },
                {
                    "id": "groq/llama3-8b-8192",
                    "provider": "groq",
                    "actual_model_name": "llama3-8b-8192",
                    "description": "Llama 3 8B on Groq - Very fast",
                    "free_tier_available": True,
                    "context_window": 8192
                }
            ]
        }
        save_app_config(default_config)
        return default_config
    try:
        return json.loads(CONFIG_FILE.read_text())
    except json.JSONDecodeError:
        default_config = {"service_version": "0.1.0", "models": []}
        save_app_config(default_config)
        return default_config

def save_app_config(data: Dict[str, Any]):
    CONFIG_FILE.write_text(json.dumps(data, indent=2))

app_config_on_startup = load_app_config()

class Message(BaseModel):
    role: str
    content: str

class ChatCompletionRequest(BaseModel):
    model: str = Field(..., description="The model identifier, e.g., 'google/gemini-1.5-flash-latest'")
    messages: List[Message]
    max_tokens: Optional[int] = None
    temperature: Optional[float] = Field(None, ge=0.0, le=2.0)
    stream: Optional[bool] = False

class ChatCompletionChoice(BaseModel):
    index: int
    message: Message
    finish_reason: Optional[str] = None

class Usage(BaseModel):
    prompt_tokens: Optional[int] = None
    completion_tokens: Optional[int] = None
    total_tokens: Optional[int] = None

class ChatCompletionResponse(BaseModel):
    id: str
    object: str = "chat.completion"
    created: int
    model: str
    choices: List[ChatCompletionChoice]
    usage: Optional[Usage] = None
    system_fingerprint: Optional[str] = None

class ModelInfo(BaseModel):
    id: str
    provider: str
    actual_model_name: str
    description: str
    free_tier_available: bool
    context_window: Optional[int] = None

class GatewayConfigResponse(BaseModel):
    service_version: str
    default_max_tokens: int
    models: List[ModelInfo]

class ContextResponse(BaseModel):
    relic_name: str
    version: str
    description: str
    system_prompt_fragment: str
    capabilities: List[str]
    status_notes: str

@app.get("/context", response_model=ContextResponse)
async def get_context_endpoint():
    current_config = load_app_config()
    return {
        "relic_name": "ai-inference-gateway",
        "version": current_config.get("service_version", "0.1.0"),
        "description": "AI Inference API Gateway to route requests to various free-tier LLM providers.",
        "system_prompt_fragment": "This gateway accepts OpenAI-compatible /v1/chat/completions requests and routes them. Specify model using 'provider/model-name' format.",
        "capabilities": [
            "Provides gateway context via /context",
            "Lists available models and gateway config via /api/v1/config",
            "Routes chat completion requests via /v1/chat/completions to configured LLM providers (Gemini, Groq)"
        ],
        "status_notes": "Operational. Ready to proxy LLM requests. Ensure API keys are set in .env."
    }

@app.get("/api/v1/config", response_model=GatewayConfigResponse)
async def get_gateway_config():
    return load_app_config()

@app.post("/v1/chat/completions", response_model=ChatCompletionResponse)
async def chat_completions(payload: ChatCompletionRequest):
    gateway_config = load_app_config()
    model_config = next((m for m in gateway_config.get("models", []) if m["id"] == payload.model), None)

    if not model_config:
        raise HTTPException(status_code=404, detail=f"Model '{payload.model}' not found or not configured.")

    provider = model_config.get("provider")
    actual_model_name = model_config.get("actual_model_name", payload.model)
    
    try:
        if provider == "gemini":
            api_key = os.getenv("GEMINI_API_KEY")
            if not api_key:
                raise HTTPException(status_code=500, detail="GEMINI_API_KEY not configured.")
            response_content, usage_stats = await call_gemini_api(
                model_name=actual_model_name, 
                messages=payload.messages, 
                api_key=api_key,
                max_tokens=payload.max_tokens or gateway_config.get("default_max_tokens"),
                temperature=payload.temperature
            )
        elif provider == "groq":
            api_key = os.getenv("GROQ_API_KEY")
            if not api_key:
                raise HTTPException(status_code=500, detail="GROQ_API_KEY not configured.")
            response_content, usage_stats = await call_groq_api(
                model_name=actual_model_name, 
                messages=payload.messages, 
                api_key=api_key,
                max_tokens=payload.max_tokens or gateway_config.get("default_max_tokens"),
                temperature=payload.temperature
            )
        else:
            raise HTTPException(status_code=501, detail=f"Provider '{provider}' not implemented.")

        return ChatCompletionResponse(
            id=f"chatcmpl-gateway-{os.urandom(8).hex()}",
            created=int(time.time()),
            model=payload.model,
            choices=[
                ChatCompletionChoice(
                    index=0,
                    message=Message(role="assistant", content=response_content),
                    finish_reason="stop"
                )
            ],
            usage=usage_stats
        )

    except httpx.HTTPStatusError as e:
        raise HTTPException(status_code=e.response.status_code, detail=f"Error from {provider}: {e.response.text}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error processing request for {provider}: {str(e)}")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=int(os.getenv("BACKEND_PORT_CONTAINER", 8000)), reload=True)
