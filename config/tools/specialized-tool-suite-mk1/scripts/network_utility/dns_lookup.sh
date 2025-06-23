#!/bin/bash
PARAMS_JSON="$1"
HOSTNAME=$(echo "$PARAMS_JSON" | jq -r .hostname)
RECORD_TYPE=$(echo "$PARAMS_JSON" | jq -r .record_type // "A")
if [[ -z "$HOSTNAME" || "$HOSTNAME" == "null" ]]; then echo '{"error":"hostname parameter missing"}'; exit 1; fi

LOOKUP_RESULT=$(dig +short "$HOSTNAME" "$RECORD_TYPE")
EXIT_STATUS=$?

if [ $EXIT_STATUS -eq 0 ]; then
  if [ -n "$LOOKUP_RESULT" ]; then
    JSON_RESULT=$(echo "$LOOKUP_RESULT" | jq -R . | jq -s .)
    echo "{\"status\":\"success\", \"hostname\":\"$HOSTNAME\", \"record_type\":\"$RECORD_TYPE\", \"records\":$JSON_RESULT}"
  else
    echo "{\"status\":\"success_no_records\", \"hostname\":\"$HOSTNAME\", \"record_type\":\"$RECORD_TYPE\", \"message\":\"No records found\"}"
  fi
elif [ $EXIT_STATUS -eq 9 ]; then
    echo "{\"status\":\"host_not_found\", \"hostname\":\"$HOSTNAME\", \"record_type\":\"$RECORD_TYPE\", \"message\":\"Host not found or no such domain (NXDOMAIN)\"}"
elif [ $EXIT_STATUS -eq 1 ]; then
    echo "{\"status\":\"server_failure\", \"hostname\":\"$HOSTNAME\", \"record_type\":\"$RECORD_TYPE\", \"message\":\"DNS server failure or other lookup error (SERVFAIL or similar). Check dig output if possible.\"}"
else
    echo "{\"status\":\"error\", \"hostname\":\"$HOSTNAME\", \"record_type\":\"$RECORD_TYPE\", \"exit_code\":$EXIT_STATUS, \"message\":\"DNS lookup failed with dig exit code $EXIT_STATUS\"}"
fi
