#!/bin/bash
PARAMS_JSON="$1"
HOST=$(echo "$PARAMS_JSON" | jq -r .host)
COUNT=$(echo "$PARAMS_JSON" | jq -r .count // "4")
if [[ -z "$HOST" || "$HOST" == "null" ]]; then echo '{"error":"host parameter missing"}'; exit 1; fi
PING_OUTPUT=$(ping -c "$COUNT" "$HOST")
EXIT_STATUS=$?
if [ $EXIT_STATUS -eq 0 ]; then
  SUMMARY=$(echo "$PING_OUTPUT" | tail -n 2)
  echo "{\"status\":\"success\", \"host\":\"$HOST\", \"summary\":$(echo "$SUMMARY" | jq -R -s .), \"full_output\":$(echo "$PING_OUTPUT" | jq -R -s .)}"
elif [ $EXIT_STATUS -eq 1 ]; then
  SUMMARY=$(echo "$PING_OUTPUT" | tail -n 2)
  echo "{\"status\":\"partial_success\", \"host\":\"$HOST\", \"summary\":$(echo "$SUMMARY" | jq -R -s .), \"full_output\":$(echo "$PING_OUTPUT" | jq -R -s .)}"
else
  echo "{\"status\":\"failure\", \"host\":\"$HOST\", \"exit_code\":$EXIT_STATUS, \"error_message\":\"Ping failed or host unreachable\", \"full_output\":$(echo "$PING_OUTPUT" | jq -R -s .)}"
fi
