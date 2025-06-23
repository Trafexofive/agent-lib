# Universal AI Gateway - Setup Instructions (v0.1.0)

This server acts as an API gateway to route inference requests to various free-tier AI model providers (e.g., Gemini, Groq). **Crucially, the actual API call logic within the provider 'connectors' (`app/services_gateway/*.py`) is stubbed/simulated in this version.** You will need to implement the real API interactions and manage API keys.

## Prerequisites

- Docker & Docker Compose (v1.28+ for Compose is recommended)
- `curl` and `jq` (optional, for easier JSON viewing from terminal)
- API keys for any free-tier AI services you intend to integrate (e.g., Google AI Studio for Gemini, GroqCloud).

## 1. Materialize Project

If you have this plan as a JSON file (e.g., `universal_ai_gateway_plan.json`), use your `relic_materializer.py` script:

```bash
python relic_materializer.py universal_ai_gateway_plan.json --output-dir ./my_gateways --force
cd ./my_gateways/universal-ai-gateway
```
This creates the `universal-ai-gateway/` directory.

## 2. Environment Configuration (`.env`)

1.  The `make validate-env` target (called by `make up`) will prompt you to copy `.env.example` to `.env` if `.env` is missing.
2.  **CRITICAL**: Edit the newly created `.env` file. You **MUST** fill in the API keys for the services you want to use:
    ```env
    # Universal AI Gateway Configuration
    APP_PORT=8011
    # ... other settings ...

    # --- Provider Specific Configurations (User MUST fill these for actual use) --- 
    GEMINI_API_KEY="YOUR_GEMINI_API_KEY_HERE"
    GROQ_API_KEY="YOUR_GROQ_API_KEY_HERE"
    # OTHER_FREE_PROVIDER_API_KEY="YOUR_OTHER_KEY"
    ```
    - If API keys are not provided or left as placeholders, the respective connectors will run in a fully **simulated** mode.
    - Review `CORS_ALLOWED_ORIGINS` if you plan to call this gateway from a frontend application on a different domain/port.

## 3. Build and Run with Docker

1.  **Build the Docker image**:
    ```bash
    make build
    ```
2.  **Start the application services**:
    ```bash
    make up
    ```
    The API gateway should now be running. The `make up` command will indicate the URL, typically `http://localhost:8011` (or your configured `APP_PORT`).

## 4. Interacting with the API

The primary endpoint for chat is `POST /api/v1/inference/chat/completions`.

**System Endpoints:**
```bash
curl http://localhost:8011/system/health | jq
curl http://localhost:8011/system/capabilities | jq
```

**Example Chat Completion Request (using `curl`):**

Replace `PROVIDER_NAME` with `gemini` or `groq`. Replace `MODEL_NAME` if desired (or omit to use default for the provider).

```bash
# Example for Gemini (simulated if API key is placeholder)
curl -X POST -H "Content-Type: application/json" -d '\
{\
  "provider": "gemini",\
  "model": "gemini-1.5-flash-latest", \
  "messages": [\
    {"role": "user", "content": "Explain quantum entanglement in simple terms."}\
  ],\
  "max_tokens": 100,\
  "temperature": 0.7\
}' http://localhost:8011/api/v1/inference/chat/completions | jq

# Example for Groq (simulated if API key is placeholder)
curl -X POST -H "Content-Type: application/json" -d '\
{\
  "provider": "groq",\
  "model": "llama3-8b-8192", \
  "messages": [\
    {"role": "user", "content": "Write a short poem about a fast AI."}\
  ],\
  "max_tokens": 60,\
  "temperature": 0.8\
}' http://localhost:8011/api/v1/inference/chat/completions | jq
```

## 5. Implementing Real AI Provider Connectors

This is the main task left for the user:

1.  **Obtain API Keys:** Sign up for the free tiers of services like Google AI Studio (for Gemini) or GroqCloud and get your API keys.
2.  **Update `.env`:** Put your actual API keys into the `.env` file.
3.  **Install SDKs (if necessary):**
    *   If a provider has a Python SDK (e.g., `google-generativeai` for Gemini), add it to `requirements.txt` and run `make build` again.
    *   For providers with simple REST APIs (like Groq's OpenAI-compatible endpoint), `httpx` (already included) is sufficient.
4.  **Modify Connector Logic:**
    *   Go to `app/services_gateway/gemini_connector.py` (for Gemini), `app/services_gateway/groq_connector.py` (for Groq), etc.
    *   Remove or comment out the `SIMULATION BLOCK`.
    *   Uncomment and complete the `_make_http_request` or SDK-specific call logic.
    *   Ensure the payload sent to the provider matches their API specification.
    *   Implement the `_parse_PROVIDER_response` method to correctly transform the provider's native response into the gateway's `ChatCompletionResponse` Pydantic model. Pay close attention to message structures, token counts, and finish reasons.
    *   Handle provider-specific errors gracefully.

## 6. Stopping and Cleaning Up

-   **Stop services**: `make down`
-   **Full cleanup** (removes containers, and the Docker image):
    ```bash
    make clean
    ```

## Development Notes

-   Logs: `make logs`.
-   The gateway attempts to standardize requests and responses, but you'll need to consult the documentation for each specific AI provider's free tier to understand their exact capabilities, rate limits, and data formats.
-   Implementing robust error handling, retry logic, and accurate rate limit tracking for multiple free APIs is a complex task beyond this initial stub.