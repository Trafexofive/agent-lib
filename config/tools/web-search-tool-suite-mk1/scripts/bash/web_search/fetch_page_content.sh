#!/bin/bash
PARAMS_JSON="$1"
URL=$(echo "$PARAMS_JSON" | jq -r .url)
USER_AGENT=$(echo "$PARAMS_JSON" | jq -r .user_agent // "AgentShellCurl/1.0 (FetchPageContentTool)")

if [[ -z "$URL" || "$URL" == "null" ]]; then echo '{"error":"url parameter missing"}'; exit 1; fi

# Temporary file to store output and status code
TMP_OUTPUT=$(mktemp)

# Perform curl command, get body and HTTP status code
# -L: follow redirects
# -A: set User-Agent
# -k: allow insecure SSL connections (useful for dev, consider removing for production or specific needs)
# --connect-timeout 10: timeout for connection
# --max-time 20: max time for total operation
HTTP_STATUS=$(curl -s -L -k -A "$USER_AGENT" --connect-timeout 10 --max-time 20 -o "$TMP_OUTPUT" -w "%{http_code}" "$URL")
CURL_EXIT_CODE=$?

if [ $CURL_EXIT_CODE -ne 0 ]; then
  rm -f "$TMP_OUTPUT"
  echo "{\"error\":\"curl_command_failed\", \"url\":\"$URL\", \"curl_exit_code\":$CURL_EXIT_CODE, \"message\":\"Curl command failed. Check network or URL validity.\"}"
  exit 1
fi

CONTENT=$(cat "$TMP_OUTPUT")
rm -f "$TMP_OUTPUT"

if [ "$HTTP_STATUS" -ge 200 ] && [ "$HTTP_STATUS" -lt 400 ]; then
  # Basic check if content looks like HTML (very naive)
  IS_HTML=false
  if echo "$CONTENT" | grep -q -i -E '<html|<head|<body'; then
    IS_HTML=true
  fi 
  # Escape content for JSON embedding
  ESCAPED_CONTENT=$(echo "$CONTENT" | jq -R -s .)
  echo "{\"status\":\"success\", \"url\":\"$URL\", \"http_status_code\":$HTTP_STATUS, \"content_length_bytes\":${#CONTENT}, \"likely_html\":$IS_HTML, \"content\":$ESCAPED_CONTENT}"
elif [ "$HTTP_STATUS" -eq 0 ]; then # Curl sometimes returns 000 for certain errors like timeout, even if curl_exit_code is 0
    echo "{\"error\":\"fetch_failed_unknown_status\", \"url\":\"$URL\", \"http_status_code\":0, \"message\":\"Fetch attempt resulted in HTTP status 000, possibly a timeout or DNS issue despite curl exit code 0. Content might be empty or incomplete.\"}"
else
  ESCAPED_CONTENT=$(echo "$CONTENT" | jq -R -s .)
  echo "{\"error\":\"fetch_failed_http_error\", \"url\":\"$URL\", \"http_status_code\":$HTTP_STATUS, \"content_length_bytes\":${#CONTENT}, \"content\":$ESCAPED_CONTENT, \"message\":\"Failed to fetch URL, HTTP status indicates error.\"}"
fi
