#!/bin/bash
source "$(dirname "$0")/common_vars.sh"
PARAMS_JSON="$1"
# ... rest of script from previous response ...
PLAN_JSON_STRING=$(echo "$PARAMS_JSON" | jq -r .plan_json_string)
# ... rest of script ...
call_api "POST" "/forge/relic" "$TMP_PLAN_FILE"
exit $?