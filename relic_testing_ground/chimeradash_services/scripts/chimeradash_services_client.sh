#!/bin/bash

BACKEND_HOST="localhost"
BACKEND_PORT=${BACKEND_PORT:-8000} # Use environment variable or default to 8000

BASE_URL="http://${BACKEND_HOST}:${BACKEND_PORT}"

echo "ChimeraDash Services Relic Client"
echo "=================================="
echo "Using backend API at: ${BASE_URL}"

echo "\n--- Fetching /context --- "
curl -s -X GET "${BASE_URL}/context" | jq '.'

echo "\n\n--- Fetching current /api/v1/config --- "
curl -s -X GET "${BASE_URL}/api/v1/config" | jq '.'

CONFIG_VALUE_BEFORE=$(curl -s -X GET "${BASE_URL}/api/v1/config" | jq -r '.sample_setting_1')
NEW_VALUE="updated_by_client_script_$(date +%s)"

echo "\n\n--- Attempting to update /api/v1/config (sample_setting_1 to '${NEW_VALUE}') --- "
curl -s -X POST "${BASE_URL}/api/v1/config" \
     -H "Content-Type: application/json" \
     -d "{\"sample_setting_1\": \"${NEW_VALUE}\"}" | jq '.'

echo "\n\n--- Fetching /api/v1/config again to verify update --- "
curl -s -X GET "${BASE_URL}/api/v1/config" | jq '.'

CONFIG_VALUE_AFTER=$(curl -s -X GET "${BASE_URL}/api/v1/config" | jq -r '.sample_setting_1')

if [ "${CONFIG_VALUE_AFTER}" == "${NEW_VALUE}" ]; then
    echo "\nSUCCESS: sample_setting_1 was updated."
else
    echo "\nWARNING: sample_setting_1 was NOT updated as expected. Before: '${CONFIG_VALUE_BEFORE}', Expected After: '${NEW_VALUE}', Actual After: '${CONFIG_VALUE_AFTER}'"
fi

echo "\n\n--- Fetching setting 'sample_setting_1' directly from DB via API --- "
curl -s -X GET "${BASE_URL}/api/v1/settings/sample_setting_1" | jq '.'

echo "\nClient script finished."
