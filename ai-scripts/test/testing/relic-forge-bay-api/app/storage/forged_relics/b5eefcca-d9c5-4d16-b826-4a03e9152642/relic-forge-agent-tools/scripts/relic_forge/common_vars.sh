#!/bin/bash
# common_vars.sh
# Sourced by other scripts to set RELIC_FORGE_BASE_URL and common helper functions.

# Use environment variable if set, otherwise default
: "${RELIC_FORGE_BASE_URL:=http://localhost:8000}" # Matches relic-forge-bay-api default

# Ensure jq and curl are available
ensure_commands() {
    local missing_cmds=0
    if ! command -v jq &> /dev/null; then
        echo "{\"error\": \"jq command not found. Please install jq.\"}"
        missing_cmds=1
    fi
    if ! command -v curl &> /dev/null; then
        echo "{\"error\": \"curl command not found. Please install curl.\"}"
        missing_cmds=1
    fi
    if [ "$missing_cmds" -eq 1 ]; then
        exit 1 # Critical failure if commands are missing
    fi
}
ensure_commands

# Function to make curl requests and handle basic output/errors
# Usage: call_api METHOD URL_PATH [DATA_FILE_PATH]
# URL_PATH is relative to RELIC_FORGE_BASE_URL (e.g., "/forge/relics")
# DATA_FILE_PATH is optional, used for POST/PUT with -d @file
call_api() {
    local method="$1"
    local url_path="$2"
    local data_file_path="$3"
    local full_url="${RELIC_FORGE_BASE_URL}${url_path}"
    
    local response_output
    local http_status
    # -L to follow redirects, -k for insecure SSL (dev only, remove if not needed)
    local curl_options=(-s -L -w "\n%{http_code}") 

    if [ -n "$data_file_path" ]; then
        response_output=$(curl "${curl_options[@]}" -X "$method" -H "Content-Type: application/json" -d "@$data_file_path" "$full_url")
    else
        response_output=$(curl "${curl_options[@]}" -X "$method" "$full_url")
    fi
    
    local curl_exit_code=$?
    http_status=$(echo "$response_output" | tail -n1)
    body=$(echo "$response_output" | sed '$d')

    if [ $curl_exit_code -ne 0 ]; then
        echo "{\"error\": \"curl_command_failed\", \"curl_exit_code\": $curl_exit_code, \"method\": \"$method\", \"url\": \"$full_url\", \"details\": \"$(echo "$body" | tr -d '\n\r' | sed 's/\"/\\\"/g')\"}"
        return 1
    fi

    if [ "$http_status" -ge 200 ] && [ "$http_status" -lt 300 ]; then
        if [ -z "$body" ] && [ "$http_status" -eq 204 ]; then
             echo "{\"status\": \"success\", \"http_status\": $http_status, \"message\": \"Operation successful, no content returned.\"}"
        elif echo "$body" | jq -e . > /dev/null 2>&1; then
            echo "$body" | jq '.' # Output formatted JSON
        else
            # Body is not JSON, output as plain text or simple JSON wrapper
            # This case might occur if API unexpectedly returns non-JSON on success
            echo "{\"status\": \"success_non_json_body\", \"http_status\": $http_status, \"response_body\": \"$(echo "$body" | tr -d '\n\r' | sed 's/\"/\\\"/g')\"}"
        fi
        return 0
    else
        # API returned an error status code
        if echo "$body" | jq -e . > /dev/null 2>&1; then
            echo "$body" | jq '.' # API error is JSON
        else
            # API error is not JSON
            echo "{\"error\": \"api_error_non_json_body\", \"http_status\": $http_status, \"method\": \"$method\", \"url\": \"$full_url\", \"response_body\": \"$(echo "$body" | tr -d '\n\r' | sed 's/\"/\\\"/g')\"}"
        fi
        return 1
    fi
}
