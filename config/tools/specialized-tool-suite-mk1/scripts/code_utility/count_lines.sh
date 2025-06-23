#!/bin/bash
PARAMS_JSON="$1"
FILE_PATH=$(echo "$PARAMS_JSON" | jq -r .file_path)
if [ -z "$FILE_PATH" ] || [ "$FILE_PATH" == "null" ]; then echo '{"error":"file_path parameter missing"}'; exit 1; fi
if [ ! -f "$FILE_PATH" ]; then echo "{\"error\":\"File not found: $FILE_PATH\"}"; exit 1; fi

WC_OUTPUT=$(wc "$FILE_PATH")
LINES=$(echo "$WC_OUTPUT" | awk '{print $1}')
WORDS=$(echo "$WC_OUTPUT" | awk '{print $2}')
BYTES=$(echo "$WC_OUTPUT" | awk '{print $3}')

echo "{\"file\":\"$FILE_PATH\", \"lines\":$(echo $LINES | jq .), \"words\":$(echo $WORDS | jq .), \"bytes\":$(echo $BYTES | jq .)}"
