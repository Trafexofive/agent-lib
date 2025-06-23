#!/bin/bash
# Client for ai-inference-gateway (v0.1.0)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ENV_FILE_PROJECT_ROOT="$SCRIPT_DIR/../.env"

if [ -f "$ENV_FILE_PROJECT_ROOT" ]; then
    source "$ENV_FILE_PROJECT_ROOT"
else
    echo "Warning: .env file not found at project root. API keys might be missing for tests."
fi

BACKEND_PORT_HOST="${BACKEND_PORT_HOST:-8002}"
GATEWAY_URL="http://localhost:${BACKEND_PORT_HOST}"

echo "AI Inference Gateway Client - Relic: ai-inference-gateway"
echo "======================================================"
echo "API Gateway URL: ${GATEWAY_URL}"

echo "\n--- [1] Fetching /context --- "
curl -s -X GET "${GATEWAY_URL}/context" | jq '.' || echo "Error fetching /context. Is jq installed? Is the backend running?"

echo "\n\n--- [2] Fetching gateway configuration (/api/v1/config) --- "
curl -s -X GET "${GATEWAY_URL}/api/v1/config" | jq '.' || echo "Error fetching /api/v1/config."

# --- Test with Gemini ---
echo "\n\n--- [3] Testing Chat Completion with Gemini (google/gemini-1.5-flash-latest) --- "
if [ -z "$GEMINI_API_KEY" ] || [ "$GEMINI_API_KEY" == "YOUR_GOOGLE_GEMINI_API_KEY_HERE" ]; then
    echo "GEMINI_API_KEY not set or is placeholder in .env. Skipping Gemini test."
else
    echo "Using GEMINI_API_KEY from .env for test."
    curl -s -X POST "${GATEWAY_URL}/v1/chat/completions" \
         -H "Content-Type: application/json" \
         -d '{
               "model": "google/gemini-1.5-flash-latest",
               "messages": [
                 {"role": "system", "content": "You are a helpful assistant."},
                 {"role": "user", "content": "What is the capital of France? Answer in one word."}
               ],
               "max_tokens": 50
             }' | jq '.' || echo "Error during Gemini chat completion test."
fi

# --- Test with Groq ---
echo "\n\n--- [4] Testing Chat Completion with Groq (groq/llama3-8b-8192) --- "
if [ -z "$GROQ_API_KEY" ] || [ "$GROQ_API_KEY" == "YOUR_GROQ_API_KEY_HERE" ]; then
    echo "GROQ_API_KEY not set or is placeholder in .env. Skipping Groq test."
else
    echo "Using GROQ_API_KEY from .env for test."
    curl -s -X POST "${GATEWAY_URL}/v1/chat/completions" \
         -H "Content-Type: application/json" \
         -d '{
               "model": "groq/llama3-8b-8192",
               "messages": [
                 {"role": "user", "content": "Explain the concept of a Large Language Model in one short sentence."}
               ],
               "max_tokens": 100,
               "temperature": 0.7
             }' | jq '.' || echo "Error during Groq chat completion test."
fi

echo "\n\nClient script finished. Remember to populate 'chimeradash_frontend_code/' with your frontend project."
