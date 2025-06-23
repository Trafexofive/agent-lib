# Chimera Relic: ai-inference-gateway - Agent Interaction & Development Guidelines

**Version:** `0.1.0`

This document provides guidelines for interacting with and modifying the **`ai-inference-gateway`** relic.

## 1. Agent Interaction Protocol

### 1.1 Key Components

*   **`ai-inference-gateway-backend-api`:** The core API gateway.
    *   Manages routing to different LLM providers based on the `model` field in requests.
    *   Requires API keys for providers to be set in the `.env` file.
*   **`ai-inference-gateway-frontend-ui`:** Serves your custom ChimeraDash OpenRouter-style frontend.
    *   **USER ACTION:** You must populate `chimeradash_frontend_code/` with your frontend project.

### 1.2 Agent Context Endpoint (`/context`)

*   **URL:** `http://ai-inference-gateway-backend-api:8000/context`
*   **Method:** `GET`
*   **Provides:** Relic name, version, capabilities, status, and a system prompt fragment for AI interaction.

### 1.3 Gateway Configuration Endpoint (`/api/v1/config`)

*   **URL:** `http://ai-inference-gateway-backend-api:8000/api/v1/config`
*   **Method:** `GET`
*   **Response Example:**
    ```json
    {
      "service_version": "0.1.0",
      "default_max_tokens": 2048,
      "models": [
        {
          "id": "google/gemini-1.5-flash-latest",
          "provider": "gemini",
          "actual_model_name": "gemini-1.5-flash-latest",
          "description": "Google Gemini Flash - Fast and versatile",
          "free_tier_available": true,
          "context_window": 8192
        }
        // ... other models ...
      ]
    }
    ```

### 1.4 Chat Completions Endpoint (`/v1/chat/completions`)

This endpoint is designed to be OpenAI-compatible.

*   **URL:** `http://ai-inference-gateway-backend-api:8000/v1/chat/completions`
*   **Method:** `POST`
*   **Request Body Example (OpenAI format):
    ```json
    {
      "model": "google/gemini-1.5-flash-latest", // Or "groq/llama3-8b-8192", etc.
      "messages": [
        {"role": "system", "content": "You are a concise assistant."},
        {"role": "user", "content": "What is the capital of France?"}
      ],
      "max_tokens": 50,
      "temperature": 0.7
    }
    ```
*   **Response Example (OpenAI format):
    ```json
    {
      "id": "chatcmpl-gateway-xxxx",
      "object": "chat.completion",
      "created": 1677652288,
      "model": "google/gemini-1.5-flash-latest",
      "choices": [
        {
          "index": 0,
          "message": {
            "role": "assistant",
            "content": "Paris"
          },
          "finish_reason": "stop"
        }
      ],
      "usage": {
        "prompt_tokens": null,      // Placeholder for v0.1 Gemini
        "completion_tokens": null,  // Placeholder for v0.1 Gemini
        "total_tokens": null        // Placeholder for v0.1 Gemini
      }
    }
    ```

## 2. Feature Checklist

*   [X] Backend API Gateway (`ai-inference-gateway-backend-api`)
    *   [X] `/context` endpoint.
    *   [X] `/api/v1/config` endpoint to list models and basic gateway config.
    *   [X] `/v1/chat/completions` endpoint (OpenAI compatible).
    *   [X] Integration with Google Gemini (via `google-generativeai` SDK).
    *   [X] Integration with Groq (via `groq` SDK).
    *   [X] API key management via `.env` file.
*   [X] Frontend Service Container (`ai-inference-gateway-frontend-ui`)
    *   [X] Dockerfile to build & serve user's Vite/React app via Nginx.
    *   [X] `VITE_API_GATEWAY_URL` environment variable for backend communication.
*   [X] Standard DevOps (`Makefile`, `docker-compose.yml`, `.env`).

## 3. Modification Guidelines

### 3.1 Project Structure

```
ai-inference-gateway/
├── backend_api_service/      # FastAPI API Gateway
│   ├── Dockerfile
│   ├── main.py               # Main routing and OpenAI compatibility layer
│   ├── llm_providers.py      # Logic for individual LLM providers (Gemini, Groq)
│   ├── requirements.txt
│   └── app_config.json       # Model list and gateway settings
├── chimeradash_frontend_code/  # USER PROVIDES THEIR FRONTEND PROJECT HERE
│   ├── Dockerfile            # Builds and serves the frontend (Vite + Nginx)
│   ├── nginx.conf
│   ├── vite-env.d.ts         # Vite type declarations
│   ├── (user's package.json, vite.config.ts, src/, etc.)
├── scripts/
│   └── ai_gateway_client.sh
├── .env
├── .gitignore
├── docker-compose.yml
├── Makefile
└── GUIDELINES.md
```

### 3.2 Adding New LLM Providers

1.  Add the provider's Python SDK to `backend_api_service/requirements.txt`.
2.  Create a new function in `backend_api_service/llm_providers.py` (e.g., `call_new_provider_api(...)`).
    *   This function should accept parameters like `model_name`, `messages`, `api_key`, `max_tokens`, `temperature`.
    *   It should make the API call to the new provider.
    *   It must return a tuple: `(response_text: str, usage_stats: Usage)`. Handle token counting if available, otherwise return placeholders for `Usage`.
3.  Update `backend_api_service/main.py`:
    *   Import your new function from `llm_providers.py`.
    *   Add an `elif provider == "new_provider_name":` block in the `/v1/chat/completions` endpoint to call your new function.
    *   Add the new provider's API key to `.env` (e.g., `NEW_PROVIDER_API_KEY=...`).
4.  Add model configurations for the new provider to `backend_api_service/app_config.json` under the `"models"` array.
5.  Rebuild the backend: `make build service=ai-inference-gateway-backend-api`.

### 3.3 Frontend Integration

*   **CRITICAL:** Populate `chimeradash_frontend_code/` with your full ChimeraDash frontend project.
*   Ensure your frontend reads `import.meta.env.VITE_API_GATEWAY_URL` to get the backend API endpoint.
*   The frontend should make requests to `/v1/chat/completions` of the gateway.
*   It can fetch `/api/v1/config` from the gateway to dynamically list available models.

## 4. API Key Management

*   All API keys for LLM providers are managed through the `.env` file at the root of the relic.
*   Ensure you have valid free-tier (or paid, if applicable) API keys for each provider you intend to use (Gemini, Groq, etc.) and update them in `.env`.

---
*Forge wisely, Architect.*