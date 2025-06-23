#!/bin/bash
# Client for Generic Automation Platform (v1.0.0)
set -a; source .env; set +a
BACKEND_BASE_URL="http://localhost:${BACKEND_PORT_HOST}"
echo "### Querying Relic Context Endpoint ###"
curl -s -X GET "${BACKEND_BASE_URL}/context" | jq .
echo -e "\n### Reloading components (live test) ###"
curl -s -X GET "${BACKEND_BASE_URL}/api/v1/components/reload" | jq .
