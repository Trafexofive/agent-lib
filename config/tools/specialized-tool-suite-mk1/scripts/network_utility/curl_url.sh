#!/bin/bash
PARAMS_JSON="$1"
URL=$(echo "$PARAMS_JSON" | jq -r .url)
METHOD=$(echo "$PARAMS_JSON" | jq -r .method // "GET")
DATA_PAYLOAD=$(echo "$PARAMS_JSON" | jq -r .data // "null")
HEADERS_JSON=$(echo "$PARAMS_JSON" | jq -r .headers // "{}")
OUTPUT_FORMAT=$(echo "$PARAMS_JSON" | jq -r .output_format // "text")

if [[ -z "$URL" || "$URL" == "null" ]]; then echo '{"error":"url parameter missing"}'; exit 1; fi

CURL_CMD=("curl" "-s" "-L" "-w" "\nHTTP_STATUS_CODE:%{http_code}" "-X" "$METHOD")

if [ -n "$HEADERS_JSON" ] && [ "$HEADERS_JSON" != "{}" ]; then
  for key in $(echo "$HEADERS_JSON" | jq -r 'keys[]'); do
    value=$(echo "$HEADERS_JSON" | jq -r ".$key")
    CURL_CMD+=("-H" "$key: $value")
  done
fi

if [ "$DATA_PAYLOAD" != "null" ] && [ -n "$DATA_PAYLOAD" ]; then
  CURL_CMD+=("-d" "$DATA_PAYLOAD")
  if ! echo "$HEADERS_JSON" | jq -e '. | map(select(test("Content-Type"; "i"))) | length > 0' > /dev/null; then
    if echo "$DATA_PAYLOAD" | jq -e . > /dev/null 2>&1; then
        CURL_CMD+=("-H" "Content-Type: application/json")
    fi
  fi
fi

CURL_CMD+=("$URL")

RESPONSE_WITH_STATUS=$("${CURL_CMD[@]}")
CURL_EXIT_CODE=$?

HTTP_STATUS=$(echo "$RESPONSE_WITH_STATUS" | grep "^HTTP_STATUS_CODE:" | cut -d':' -f2)
BODY=$(echo "$RESPONSE_WITH_STATUS" | sed '$d')

if [ $CURL_EXIT_CODE -ne 0 ]; then
  echo "{\"error\":\"curl_command_failed\", \"exit_code\":$CURL_EXIT_CODE, \"url\":\"$URL\"}"; exit 1;
fi

PARSED_BODY="null"
if [ "$OUTPUT_FORMAT" == "json" ]; then
  if echo "$BODY" | jq -e . > /dev/null 2>&1; then
    PARSED_BODY=$(echo "$BODY" | jq -c .)
  else
    PARSED_BODY=$(echo "$BODY" | jq -R -s . )
  fi
elif [ -n "$BODY" ]; then
  PARSED_BODY=$(echo "$BODY" | jq -R -s .)
fi

echo "{\"http_status_code\":$(echo "$HTTP_STATUS" | jq .), \"body\":$PARSED_BODY, \"curl_exit_code\":$CURL_EXIT_CODE}"
