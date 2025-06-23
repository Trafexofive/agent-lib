#!/bin/bash
PARAMS_JSON="$1"
CHECK_PATH=$(echo "$PARAMS_JSON" | jq -r .path)
if [ -z "$CHECK_PATH" ] || [ "$CHECK_PATH" == "null" ]; then echo '{"error":"path parameter missing"}'; exit 1; fi
if [ -e "$CHECK_PATH" ]; then
  if [ -f "$CHECK_PATH" ]; then TYPE="file"; else TYPE="directory"; fi
  echo "{\"exists\":true, \"path\":\"$CHECK_PATH\", \"type\":\"$TYPE\"}"
else
  echo "{\"exists\":false, \"path\":\"$CHECK_PATH\", \"type\":\"not_found\"}"
fi
