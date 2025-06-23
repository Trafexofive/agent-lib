#!/bin/bash
# forge_relic.sh
# Params JSON: {"plan_json_string": "JSON string of the relic plan"}

# Source common variables and functions
source "$(dirname "$0")/common_vars.sh"

PARAMS_JSON="$1"

if [ -z "$PARAMS_JSON" ]; then
  echo "{\"error\": \"parameter_missing\", \"message\": \"No parameters JSON string provided to forge_relic.sh.\"}"
  exit 1
fi

PLAN_JSON_STRING=$(echo "$PARAMS_JSON" | jq -r .plan_json_string)

if [ "$PLAN_JSON_STRING" == "null" ] || [ -z "$PLAN_JSON_STRING" ]; then
    echo "{\"error\": \"parameter_invalid\", \"message\": \"'plan_json_string' field missing or empty in JSON parameters.\"}"
    exit 1
fi

# Create a temporary file for the plan
TMP_PLAN_FILE=$(mktemp)
if [ -z "$TMP_PLAN_FILE" ]; then
    echo "{\"error\": \"tempfile_creation_failed\", \"message\": \"Failed to create temporary file for plan.\"}"
    exit 1
fi
# Ensure cleanup of temp file on script exit
trap 'rm -f "$TMP_PLAN_FILE"' EXIT

# Write the plan string to the temporary file
# Check if plan_json_string is valid JSON before writing
if ! echo "$PLAN_JSON_STRING" | jq -e . > /dev/null; then
    echo "{\"error\": \"parameter_invalid\", \"message\": \"'plan_json_string' is not valid JSON.\"}"
    exit 1
fi
echo "$PLAN_JSON_STRING" > "$TMP_PLAN_FILE"

# Make the API call
call_api "POST" "/forge/relic" "$TMP_PLAN_FILE"
exit $?
