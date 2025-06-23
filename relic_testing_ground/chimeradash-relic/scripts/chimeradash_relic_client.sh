#!/bin/bash
# Client for chimeradash-relic (0.1.0)
# Load .env variables if it exists in the script's directory or parent
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ENV_FILE_PROJECT_ROOT="$SCRIPT_DIR/../.env"
ENV_FILE_SCRIPT_DIR="$SCRIPT_DIR/.env"

if [ -f "$ENV_FILE_PROJECT_ROOT" ]; then
    source "$ENV_FILE_PROJECT_ROOT"
elif [ -f "$ENV_FILE_SCRIPT_DIR" ]; then
    source "$ENV_FILE_SCRIPT_DIR"
fi

RELIC_NAME_SANITIZED="chimeradash-relic"
BACKEND_PORT_HOST="${BACKEND_PORT_HOST:-8001}"

BACKEND_BASE_URL="http://localhost:${BACKEND_PORT_HOST}"

echo "### Querying ${RELIC_NAME_SANITIZED} Agent Context Endpoint ###"
curl -s -X GET "${BACKEND_BASE_URL}/context" | jq . || echo "jq not found or error querying /context"

echo -e "\n### Querying ${RELIC_NAME_SANITIZED} Backend Configuration ###"
curl -s -X GET "${BACKEND_BASE_URL}/api/v1/config" | jq . || echo "jq not found or error querying /api/v1/config"

NEW_TITLE="ChimeraDash Updated via Client $(date +%s)"
echo -e "\n### Attempting to update dashboardTitle to '${NEW_TITLE}' ###"
curl -s -X POST "${BACKEND_BASE_URL}/api/v1/config" \
     -H "Content-Type: application/json" \
     -d "{\"dashboardTitle\": \"${NEW_TITLE}\"}" | jq .

echo -e "\n### Verifying update by fetching config again ###"
curl -s -X GET "${BACKEND_BASE_URL}/api/v1/config" | jq .

echo -e "\n### Checking 'dashboardTitle' in DB via API ###"
curl -s -X GET "${BACKEND_BASE_URL}/api/v1/db_settings/dashboardTitle" | jq .

echo -e "\nClient script completed."
