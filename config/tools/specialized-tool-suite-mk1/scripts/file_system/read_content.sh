#!/bin/bash
PARAMS_JSON="$1"
FILE_PATH=$(echo "$PARAMS_JSON" | jq -r .path)
if [ -z "$FILE_PATH" ] || [ "$FILE_PATH" == "null" ]; then echo '{"error":"path parameter missing or null"}'; exit 1; fi
if [ ! -f "$FILE_PATH" ]; then echo "{\"error\":\"File not found: $FILE_PATH\"}"; exit 1; fi
CONTENT=$(cat "$FILE_PATH")
ESCAPED_CONTENT=$(echo "$CONTENT" | jq -R -s .)
echo "{\"content\": $ESCAPED_CONTENT}"
