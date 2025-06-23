#!/bin/bash
# download_relic.sh
# Params JSON: {"relic_id": "UUID string", "output_path": "string path to save file"}

source "$(dirname "$0")/common_vars.sh"
PARAMS_JSON="$1"

if [ -z "$PARAMS_JSON" ]; then
  echo "{\"error\": \"parameter_missing\", \"message\": \"No parameters JSON string provided.\"}"
  exit 1
fi

RELIC_ID=$(echo "$PARAMS_JSON" | jq -r .relic_id)
OUTPUT_PATH=$(echo "$PARAMS_JSON" | jq -r .output_path)

if [ "$RELIC_ID" == "null" ] || [ -z "$RELIC_ID" ]; then
    echo "{\"error\": \"parameter_invalid\", \"message\": \"'relic_id' field missing or empty.\"}"
    exit 1
fi
if [ "$OUTPUT_PATH" == "null" ] || [ -z "$OUTPUT_PATH" ]; then
    echo "{\"error\": \"parameter_invalid\", \"message\": \"'output_path' field missing or empty.\"}"
    exit 1
fi

OUTPUT_DIR=$(dirname "$OUTPUT_PATH")
if ! mkdir -p "$OUTPUT_DIR"; then
    echo "{\"error\": \"directory_creation_failed\", \"message\": \"Failed to create output directory: $OUTPUT_DIR\"}"
    exit 1
fi

FULL_URL="${RELIC_FORGE_BASE_URL}/forge/relics/${RELIC_ID}/download"
HTTP_STATUS=$(curl -s -L -w "%{http_code}" -X GET "$FULL_URL" -o "$OUTPUT_PATH")
CURL_EXIT_CODE=$?

if [ $CURL_EXIT_CODE -eq 0 ]; then
    if [ "$HTTP_STATUS" -ge 200 ] && [ "$HTTP_STATUS" -lt 300 ]; then
        if [ -s "$OUTPUT_PATH" ]; then
            if file "$OUTPUT_PATH" | grep -q 'gzip compressed data'; then
                echo "{\"status\": \"success\", \"output_path\": \"$(realpath "$OUTPUT_PATH")\", \"http_status\": $HTTP_STATUS, \"message\": \"Relic downloaded successfully.\"}"
                exit 0
            else
                ERROR_CONTENT_PREVIEW=$(head -c 200 "$OUTPUT_PATH" | tr -d '\n\r' | sed 's/\"/\\\"/g')
                echo "{\"status\": \"error\", \"error_type\": \"invalid_file_type\", \"output_path\": \"$(realpath "$OUTPUT_PATH")\", \"http_status\": $HTTP_STATUS, \"message\": \"Downloaded file is not gzip. Preview: $ERROR_CONTENT_PREVIEW\"}"
                exit 1
            fi
        else
            echo "{\"status\": \"error\", \"error_type\": \"empty_file\", \"output_path\": \"$(realpath "$OUTPUT_PATH")\", \"http_status\": $HTTP_STATUS, \"message\": \"Download succeeded (HTTP $HTTP_STATUS) but output file is empty.\"}"
            exit 1
        fi
    else
        ERROR_RESPONSE_BODY=$(cat "$OUTPUT_PATH" | tr -d '\n\r' | sed 's/\"/\\\"/g')
        if echo "$ERROR_RESPONSE_BODY" | jq -e . > /dev/null 2>&1; then
             echo "$ERROR_RESPONSE_BODY" | jq '.'
        else
             echo "{\"error\": \"api_error\", \"http_status\": $HTTP_STATUS, \"message\": \"API returned HTTP error $HTTP_STATUS.\", \"response_body\": \"$ERROR_RESPONSE_BODY\"}"
        fi
        exit 1
    fi
else
    echo "{\"status\": \"error\", \"error_type\": \"curl_command_failed\", \"message\": \"curl command failed with exit code $CURL_EXIT_CODE for URL $FULL_URL.\"}"
    if [ -f "$OUTPUT_PATH" ] && [ ! -s "$OUTPUT_PATH" ]; then
        rm -f "$OUTPUT_PATH"
    fi
    exit 1
fi
