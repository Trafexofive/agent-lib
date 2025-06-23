#!/bin/bash
# Client for StorageMK1 (v0.2.1)

# Load .env variables if it exists in the script's directory or project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
ENV_FILE_PROJECT_ROOT="$SCRIPT_DIR/../.env"
ENV_FILE_SCRIPT_DIR="$SCRIPT_DIR/.env"

if [ -f "$ENV_FILE_PROJECT_ROOT" ]; then
    source "$ENV_FILE_PROJECT_ROOT"
elif [ -f "$ENV_FILE_SCRIPT_DIR" ]; then
    source "$ENV_FILE_SCRIPT_DIR"
fi

RELIC_NAME_SANITIZED="${RELIC_NAME_SANITIZED:-storagemk1}" # Fallback if not in env
BACKEND_PORT_HOST="${BACKEND_PORT_HOST:-8001}" # Fallback if not in env
DOMAIN_NAME="${DOMAIN_NAME:-localhost}"

BACKEND_BASE_URL="http://${DOMAIN_NAME}:${BACKEND_PORT_HOST}"

echo "### Querying ${RELIC_NAME_SANITIZED} Relic Context Endpoint (v0.2.1) ###"
curl -s -X GET "${BACKEND_BASE_URL}/context" | jq . || echo "jq not found or error querying /context"

echo -e "\n### Querying ${RELIC_NAME_SANITIZED} Backend Configuration ###"
curl -s -X GET "${BACKEND_BASE_URL}/api/v1/config" | jq . || echo "jq not found or error querying /api/v1/config"

echo -e "\n### Browsing root directory ('/') ###"
curl -s -X GET "${BACKEND_BASE_URL}/api/v1/storage/browse?path=/" | jq . || echo "Error browsing /"

# Example: Create a directory (uncomment and adapt to test)
# echo -e "\n### Creating a directory '/my_test_folder' ###"
# curl -s -X POST "${BACKEND_BASE_URL}/api/v1/storage/mkdir" \
#   -H "Content-Type: application/json" \
#   -d '{"path": "/my_test_folder"}' | jq .

# Example: Upload a file (create a dummy file first)
# echo "This is a test file." > test_upload.txt
# echo -e "\n### Uploading 'test_upload.txt' to '/my_test_folder' ###"
# curl -s -X POST "${BACKEND_BASE_URL}/api/v1/storage/upload?path=/my_test_folder" \
#   -F "file=@test_upload.txt" | jq .
# rm test_upload.txt

echo -e "\n### Further examples for POST/DELETE/MOVE in GUIDELINES.md or this client script."
