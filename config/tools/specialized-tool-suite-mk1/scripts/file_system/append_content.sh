#!/bin/bash
PARAMS_JSON="$1"
FILE_PATH=$(echo "$PARAMS_JSON" | jq -r .path)
CONTENT=$(echo "$PARAMS_JSON" | jq -r .content)
if [ -z "$FILE_PATH" ] || [ "$FILE_PATH" == "null" ]; then echo '{"error":"path parameter missing"}'; exit 1; fi
if [ "$CONTENT" == "null" ]; then echo '{"error":"content parameter missing"}'; exit 1; fi
mkdir -p "$(dirname "$FILE_PATH")"
if echo "$CONTENT" >> "$FILE_PATH"; then
  echo "{\"status\":\"success\", \"path\":\"$FILE_PATH\"}"
else
  echo "{\"error\":\"Failed to append to file: $FILE_PATH\"}"; exit 1
fi
