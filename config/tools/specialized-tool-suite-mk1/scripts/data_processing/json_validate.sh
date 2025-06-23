#!/bin/bash
PARAMS_JSON="$1"
JSON_STRING=$(echo "$PARAMS_JSON" | jq -r .json_string)
if [ "$JSON_STRING" == "null" ]; then echo '{"error":"json_string parameter missing or null"}'; exit 1; fi

ERROR_MSG=$(echo "$JSON_STRING" | jq -e . > /dev/null 2>&1)
JQ_EXIT_CODE=$?

if [ $JQ_EXIT_CODE -eq 0 ]; then
  echo "{\"valid\":true, \"error\":null}"
elif [ $JQ_EXIT_CODE -eq 4 ]; then
  PRECISE_ERROR=$(echo "$JSON_STRING" | jq . 2>&1 >/dev/null | tr -d '\n' | sed 's/"/\\"/g')
  echo "{\"valid\":false, \"error\":\"Invalid JSON: $PRECISE_ERROR\"}"
else
  echo "{\"valid\":false, \"error\":\"jq validation failed with exit code $JQ_EXIT_CODE (not necessarily a JSON parse error, could be jq issue or invalid input string format for jq tool itself). Consider checking input string for unescaped quotes or control characters if it's not strictly JSON.\"}"
fi
