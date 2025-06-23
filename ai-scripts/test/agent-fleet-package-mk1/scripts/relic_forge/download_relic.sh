#!/bin/bash
source "$(dirname "$0")/common_vars.sh"
PARAMS_JSON="$1"
# ... rest of script from previous response ...
RELIC_ID=$(echo "$PARAMS_JSON" | jq -r .relic_id)
OUTPUT_PATH=$(echo "$PARAMS_JSON" | jq -r .output_path)
# ... rest of script for download with curl -o ...
exit $?