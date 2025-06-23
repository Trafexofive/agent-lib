#!/bin/bash
# get_relic_metadata.sh
# Params JSON: {"relic_id": "UUID string"}

source "$(dirname "$0")/common_vars.sh"
PARAMS_JSON="$1"

if [ -z "$PARAMS_JSON" ]; then
  echo "{\"error\": \"parameter_missing\", \"message\": \"No parameters JSON string provided.\"}"
  exit 1
fi

RELIC_ID=$(echo "$PARAMS_JSON" | jq -r .relic_id)

if [ "$RELIC_ID" == "null" ] || [ -z "$RELIC_ID" ]; then
    echo "{\"error\": \"parameter_invalid\", \"message\": \"'relic_id' field missing or empty.\"}"
    exit 1
fi

call_api "GET" "/forge/relics/${RELIC_ID}"
exit $?
