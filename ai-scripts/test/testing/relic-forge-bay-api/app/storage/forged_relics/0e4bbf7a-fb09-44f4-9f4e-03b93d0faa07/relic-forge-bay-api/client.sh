#!/bin/bash

# Default Base URL - can be overridden by .env or .env.example
DEFAULT_BASE_URL="http://localhost:8000" # Script's internal default
BASE_URL="$DEFAULT_BASE_URL"

# --- Configuration & Helper Functions ---

# Detect API port from .env or .env.example in the current directory
if [ -f .env ] && grep -q -E '^FORGE_API_PORT=' .env; then
    PORT=$(grep -E '^FORGE_API_PORT=' .env | head -n1 | cut -d'=' -f2 | tr -d '[:space:]\r\n')
    if [[ "$PORT" =~ ^[0-9]+$ ]]; then 
        BASE_URL="http://localhost:$PORT"
    fi
elif [ -f .env.example ] && grep -q -E '^FORGE_API_PORT=' .env.example; then
    PORT=$(grep -E '^FORGE_API_PORT=' .env.example | head -n1 | cut -d'=' -f2 | tr -d '[:space:]\r\n')
    if [[ "$PORT" =~ ^[0-9]+$ ]]; then
        BASE_URL="http://localhost:$PORT"
    fi
fi

# RAW_OUTPUT: If set to true, curl output will not be piped to jq
: "${RAW_OUTPUT:=false}"

print_usage() {
    echo "Usage: $0 <command> [options]"
    echo "Commands:"
    echo "  forge_relic <plan_file.json>             - Forge a new relic from a JSON plan file."
    echo "  list_relics                            - List all forged relics."
    echo "  get_relic_metadata <relic_id>          - Get metadata for a specific relic."
    echo "  download_relic <relic_id> <output_file>  - Download a forged relic package."
    echo "  delete_relic <relic_id> [--force]        - Delete a forged relic. Requires --force to skip confirmation."
    echo "  get_health                             - Check system health."
    echo "  get_capabilities                       - Get system capabilities."
    echo ""
    echo "Global Options:"
    echo "  RAW_OUTPUT=true $0 <command> ...       - Disable jq formatting for the command's output."
    echo "  DEBUG=true $0 <command> ...            - Enable debug output (shows target URL, curl command)."
    echo ""
    echo "Effective API Base URL: $BASE_URL" >&2
    echo "Requires curl and jq to be installed (unless RAW_OUTPUT=true for jq)."
}

ensure_curl() {
    if ! command -v curl &> /dev/null; then
        echo "Error: curl is not installed. Please install curl to use this script." >&2
        exit 1
    fi
}

ensure_jq() {
    if [ "$RAW_OUTPUT" = "false" ] && ! command -v jq &> /dev/null; then
        echo "Error: jq is not installed. Please install jq or set RAW_OUTPUT=true." >&2
        exit 1
    fi
}

_call_api() {
    local method="$1"
    local url_path="$2"
    local data_file_path="$3"
    local expect_empty_body_on_success="$4"
    local full_url="${BASE_URL}${url_path}"
    
    if [ "$DEBUG" = "true" ]; then
        echo "DEBUG: Effective BASE_URL in _call_api: $BASE_URL" >&2
        echo "DEBUG: Attempting to call: $method $full_url" >&2
    fi
    
    local response_output
    local http_status
    local curl_options=(-s -L -w "\n%{http_code}") 

    local cmd_array=("curl" "${curl_options[@]}" "-X" "$method")
    if [ -n "$data_file_path" ]; then
        cmd_array+=("-H" "Content-Type: application/json" "-d" "@$data_file_path")
    fi
    cmd_array+=("$full_url")

    if [ "$DEBUG" = "true" ]; then
        echo "DEBUG: curl command: ${cmd_array[*]}" >&2
    fi
    
    response_output=$("${cmd_array[@]}")
    local curl_exit_code=$?
    
    http_status=$(echo -e "$response_output" | tail -n1) 
    body=$(echo -e "$response_output" | sed '$d')      

    if [ $curl_exit_code -ne 0 ]; then
        echo "Error: curl command failed with exit code $curl_exit_code for $method $full_url" >&2
        echo "Response Body (if any):" >&2
        echo "$body" >&2
        return 1
    fi

    if [ "$RAW_OUTPUT" = "true" ]; then
        echo "$body"
        echo "HTTP Status: $http_status" >&2
    else
        if [ "$http_status" -ge 200 ] && [ "$http_status" -lt 300 ]; then
            if [ "$expect_empty_body_on_success" = "true" ] && [ -z "$body" ]; then
                echo "{\"status\": \"success\", \"http_code\": $http_status, \"message\": \"Operation successful, no content returned.\"}" | jq '.'
            elif echo "$body" | jq -e . > /dev/null 2>&1; then
                echo "$body" | jq '.'
            elif [ -z "$body" ]; then 
                 echo "{\"status\": \"success_empty_body\", \"http_code\": $http_status, \"message\": \"Operation successful, empty response body.\"}" | jq '.'
            else 
                echo "Warning: API returned non-JSON response for a successful request (HTTP $http_status)." >&2
                echo "Raw Body:" >&2
                echo "$body"
            fi
        else 
            echo "Error: API responded with HTTP status $http_status for $method $full_url" >&2
            if echo "$body" | jq -e . > /dev/null 2>&1; then
                echo "Response Body (JSON):" >&2
                echo "$body" | jq '.'
            else
                echo "Response Body (Raw):" >&2
                echo "$body"
            fi
            return 1 
        fi
    fi
    return 0
}

forge_relic_cmd() {
    ensure_curl
    ensure_jq 
    local plan_file="$1"
    if [ -z "$plan_file" ]; then
        echo "Error: Plan file not specified for forge_relic." >&2
        print_usage >&2
        return 1
    fi
    if [ ! -f "$plan_file" ]; then
        echo "Error: Plan file '$plan_file' not found." >&2
        return 1
    fi
    echo "Forging relic from plan: $plan_file" >&2
    _call_api "POST" "/forge/relic" "$plan_file"
}

list_relics_cmd() {
    ensure_curl
    ensure_jq
    echo "Listing all relics" >&2
    _call_api "GET" "/forge/relics"
}

get_relic_metadata_cmd() {
    ensure_curl
    ensure_jq
    local relic_id="$1"
    if [ -z "$relic_id" ]; then
        echo "Error: Relic ID not specified for get_relic_metadata." >&2
        print_usage >&2
        return 1
    fi
    echo "Getting metadata for relic ID: $relic_id" >&2
    _call_api "GET" "/forge/relics/${relic_id}"
}

download_relic_cmd() {
    ensure_curl 
    local relic_id="$1"
    local output_file="$2"
    if [ -z "$relic_id" ]; then
        echo "Error: Relic ID not specified for download_relic." >&2
        print_usage >&2
        return 1
    fi
    if [ -z "$output_file" ]; then
        echo "Error: Output file not specified for download_relic." >&2
        print_usage >&2
        return 1
    fi
    echo "Downloading relic ID: $relic_id to $output_file" >&2
    local full_url="${BASE_URL}/forge/relics/${relic_id}/download"
    if [ "$DEBUG" = "true" ]; then
        echo "DEBUG: Effective BASE_URL for download: $BASE_URL" >&2
        echo "DEBUG: curl -s -L -X GET $full_url -o $output_file --fail" >&2
    fi
    curl -s -L -X GET "$full_url" -o "$output_file" --fail 
    local curl_exit_code=$?
    if [ $curl_exit_code -eq 0 ]; then
        if [ -f "$output_file" ] && file "$output_file" 2>/dev/null | grep -q 'gzip compressed data'; then
            echo "Download complete: $(realpath "$output_file")"
        else
            echo "Error: Downloaded file '$output_file' is not a valid gzip file or is empty." >&2
            echo "Content of (potentially failed) download at '$output_file':" >&2
            cat "$output_file" >&2 
            return 1
        fi
    else
        echo "Error: Download failed (curl exit code: $curl_exit_code)." >&2
        if [ -f "$output_file" ]; then 
             echo "Content of (potentially failed) download at '$output_file':" >&2
             cat "$output_file" >&2
        fi
        return 1
    fi
}

delete_relic_cmd() {
    ensure_curl
    ensure_jq
    local relic_id="$1"
    local force_delete="$2" 
    if [ -z "$relic_id" ]; then
        echo "Error: Relic ID not specified for delete_relic." >&2
        print_usage >&2
        return 1
    fi
    if [ "$force_delete" != "--force" ]; then
        read -r -p "Are you sure you want to delete relic '$relic_id'? This action cannot be undone. (yes/NO): " confirmation
        if [[ "$confirmation" != "yes" ]]; then
            echo "Deletion cancelled by user."
            return 0
        fi
    fi
    echo "Deleting relic ID: $relic_id" >&2
    _call_api "DELETE" "/forge/relics/${relic_id}" "" "true" 
}

get_health_cmd() {
    ensure_curl
    ensure_jq
    echo "System Health" >&2
    _call_api "GET" "/system/health"
}

get_capabilities_cmd() {
    ensure_curl
    ensure_jq
    echo "System Capabilities" >&2
    _call_api "GET" "/system/capabilities"
}

COMMAND="$1"
if [ -z "$COMMAND" ]; then
    print_usage
    exit 0
fi
shift 

case "$COMMAND" in
    forge_relic)
        forge_relic_cmd "$@"
        ;;
    list_relics)
        list_relics_cmd "$@"
        ;;
    get_relic_metadata)
        get_relic_metadata_cmd "$@"
        ;;
    download_relic)
        download_relic_cmd "$@"
        ;;
    delete_relic)
        delete_relic_cmd "$@"
        ;;
    get_health)
        get_health_cmd "$@"
        ;;
    get_capabilities)
        get_capabilities_cmd "$@"
        ;;
    help|--help|-h)
        print_usage
        ;;
    *)
        echo "Error: Unknown command '$COMMAND'" >&2
        print_usage >&2
        exit 1
        ;;
esac

exit $?
