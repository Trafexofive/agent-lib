#!/bin/bash
PARAMS_JSON="$1"
TEXT_CONTENT=$(echo "$PARAMS_JSON" | jq -r .text_content)
PATTERN=$(echo "$PARAMS_JSON" | jq -r .pattern)
IGNORE_CASE=$(echo "$PARAMS_JSON" | jq -r .ignore_case // "false")

if [ "$TEXT_CONTENT" == "null" ]; then echo '{"error":"text_content parameter missing"}'; exit 1; fi
if [[ -z "$PATTERN" || "$PATTERN" == "null" ]]; then echo '{"error":"pattern parameter missing"}'; exit 1; fi

GREP_OPTS="-E"
if [ "$IGNORE_CASE" == "true" ]; then
  GREP_OPTS+="i"
fi

mapfile -t MATCHES < <(echo "$TEXT_CONTENT" | grep "$GREP_OPTS" "$PATTERN")
COUNT=${#MATCHES[@]}

JSON_MATCHES=$(printf '%s\n' "${MATCHES[@]}" | jq -R . | jq -s .)

echo "{\"count\":$COUNT, \"matches\":$JSON_MATCHES, \"pattern\":\"$PATTERN\", \"ignore_case\":$IGNORE_CASE}"
