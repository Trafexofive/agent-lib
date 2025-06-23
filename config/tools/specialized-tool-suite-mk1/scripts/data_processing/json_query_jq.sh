#!/bin/bash
PARAMS_JSON="$1"
JSON_STRING=$(echo "$PARAMS_JSON" | jq -r .json_string)
JQ_FILTER=$(echo "$PARAMS_JSON" | jq -r .jq_filter)

if [ "$JSON_STRING" == "null" ]; then echo '{"error":"json_string parameter missing"}'; exit 1; fi
if [[ -z "$JQ_FILTER" || "$JQ_FILTER" == "null" ]]; then echo '{"error":"jq_filter parameter missing"}'; exit 1; fi

if ! echo "$JSON_STRING" | jq -e . > /dev/null 2>&1; then
  VALIDATION_ERROR=$(echo "$JSON_STRING" | jq . 2>&1 >/dev/null | tr -d '\n' | sed 's/"/\\"/g')
  echo "{\"error\":\"Invalid input JSON: $VALIDATION_ERROR\"}"
  exit 1
fi

QUERY_OUTPUT_STDERR=$( (echo "$JSON_STRING" | jq -c "$JQ_FILTER") 2>&1 )
JQ_QUERY_EXIT_CODE=$?

QUERY_OUTPUT=$(echo "$QUERY_OUTPUT_STDERR")

if [ $JQ_QUERY_EXIT_CODE -eq 0 ]; then
  if echo "$QUERY_OUTPUT" | jq -e . > /dev/null 2>&1; then
    echo "{\"result\":$QUERY_OUTPUT}"
  else
    echo "{\"result\":$(echo "$QUERY_OUTPUT" | jq -R -s .)}"
  fi
elif [ $JQ_QUERY_EXIT_CODE -eq 5 ]; then
  echo "{\"result\":null, \"info\":\"jq filter produced no value (e.g., key not found, array index out of bounds). Filter: $JQ_FILTER\"}"
elif echo "$QUERY_OUTPUT_STDERR" | jq -e . > /dev/null 2>&1; then
  echo "{\"error\":\"jq_filter_error\", \"details\":$QUERY_OUTPUT_STDERR, \"filter\":\"$JQ_FILTER\"}"
else
  echo "{\"error\":\"jq_filter_error\", \"details\":$(echo "$QUERY_OUTPUT_STDERR" | jq -R -s .), \"filter\":\"$JQ_FILTER\"}"
fi
