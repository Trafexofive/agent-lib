#!/bin/bash

DEFAULT_BASE_URL="http://localhost:8020" # Default if .env or .env.example is not found or APP_PORT is missing
BASE_URL="$DEFAULT_BASE_URL"

if [ -f .env ] && grep -q -E '^APP_PORT=' .env; then
    PORT=$(grep -E '^APP_PORT=' .env | head -n1 | cut -d'=' -f2 | tr -d '[:space:]\r\n')
    if [[ "$PORT" =~ ^[0-9]+$ ]]; then BASE_URL="http://localhost:$PORT"; fi
elif [ -f .env.example ] && grep -q -E '^APP_PORT=' .env.example; then
    PORT=$(grep -E '^APP_PORT=' .env.example | head -n1 | cut -d'=' -f2 | tr -d '[:space:]\r\n')
    if [[ "$PORT" =~ ^[0-9]+$ ]]; then BASE_URL="http://localhost:$PORT"; fi
fi

: "${RAW_OUTPUT:=false}"
: "${DEBUG:=false}"

print_usage() {
    echo "Usage: $0 <command> [options]"
    echo "Commands:"
    echo "  create_agent <agent_data.json>       - Create a new monitoring agent."
    echo "  list_agents                            - List all monitoring agents."
    echo "  get_agent <agent_id>                   - Get a specific agent by ID."
    echo "  update_agent <agent_id> <update_data.json> - Update an existing agent."
    echo "  delete_agent <agent_id> [--force]      - Delete an agent."
    echo "  get_health                             - Check system health."
    echo "  get_capabilities                       - Get system capabilities."
    echo ""
    echo "Global Options: RAW_OUTPUT=true, DEBUG=true"
    echo "Effective API Base URL: $BASE_URL" >&2
}

ensure_curl_jq() {
    if ! command -v curl &> /dev/null; then echo "Error: curl is not installed." >&2; exit 1; fi
    if [ "$RAW_OUTPUT" = "false" ] && ! command -v jq &> /dev/null; then echo "Error: jq is not installed or RAW_OUTPUT=true not set." >&2; exit 1; fi
}

_call_api() {
    local method="$1" url_path="$2" data_file="$3" expect_empty="$4"
    local full_url="${BASE_URL}${url_path}"
    local cmd_array=("curl" "-s" "-L" "-w" "\n%{http_code}" "-X" "$method")
    
    if [ "$DEBUG" = "true" ]; then echo "DEBUG: $method $full_url" >&2; fi
    if [ -n "$data_file" ]; then cmd_array+=("-H" "Content-Type: application/json" "-d" "@$data_file"); fi
    cmd_array+=("$full_url")
    if [ "$DEBUG" = "true" ]; then echo "DEBUG: curl cmd: ${cmd_array[*]}" >&2; fi

    local response_output=$("${cmd_array[@]}")
    local curl_code=$?
    local http_code=$(echo -e "$response_output" | tail -n1)
    local body=$(echo -e "$response_output" | sed '$d')

    if [ $curl_code -ne 0 ]; then echo "Error: curl failed (code $curl_code) for $method $full_url" >&2; echo "Body: $body" >&2; return 1; fi

    if [ "$RAW_OUTPUT" = "true" ]; then echo "$body"; echo "HTTP Status: $http_code" >&2; return 0; fi

    if [ "$http_code" -ge 200 ] && [ "$http_code" -lt 300 ]; then
        if [ "$expect_empty" = "true" ] && [ -z "$body" ]; then echo '{"status":"success", "http_code":'$http_code'}' | jq '.';
        elif echo "$body" | jq -e . > /dev/null 2>&1; then echo "$body" | jq '.';
        elif [ -z "$body" ]; then  echo '{"status":"success_empty_body", "http_code":'$http_code'}' | jq '.';
        else echo "Warning: Non-JSON success response (HTTP $http_code). Body: $body" >&2; fi
    else
        echo "Error: API HTTP $http_code for $method $full_url" >&2
        if echo "$body" | jq -e . > /dev/null 2>&1; then echo "Response:" >&2; echo "$body" | jq '.'; else echo "Raw Response: $body" >&2; fi
        return 1;
    fi
}

create_agent_cmd() {
    ensure_curl_jq; local file="$1"; 
    if [ -z "$file" ] || [ ! -f "$file" ]; then echo "Error: Agent data JSON file required and must exist." >&2; print_usage >&2; return 1; fi
    echo "Creating agent from: $file" >&2; _call_api "POST" "/agents" "$file"; 
}
list_agents_cmd() { ensure_curl_jq; echo "Listing agents..." >&2; _call_api "GET" "/agents"; }
get_agent_cmd() {
    ensure_curl_jq; local id="$1";
    if [ -z "$id" ]; then echo "Error: Agent ID required." >&2; print_usage >&2; return 1; fi
    echo "Getting agent ID: $id" >&2; _call_api "GET" "/agents/$id";
}
update_agent_cmd() {
    ensure_curl_jq; local id="$1" file="$2";
    if [ -z "$id" ] || [ -z "$file" ] || [ ! -f "$file" ]; then echo "Error: Agent ID and update data JSON file required." >&2; print_usage >&2; return 1; fi
    echo "Updating agent ID: $id from $file" >&2; _call_api "PUT" "/agents/$id" "$file";
}
delete_agent_cmd() {
    ensure_curl_jq; local id="$1" force="$2";
    if [ -z "$id" ]; then echo "Error: Agent ID required." >&2; print_usage >&2; return 1; fi
    if [ "$force" != "--force" ]; then read -r -p "Delete agent '$id'? (yes/NO): " conf; if [[ "$conf" != "yes" ]]; then echo "Cancelled."; return 0; fi; fi
    echo "Deleting agent ID: $id" >&2; _call_api "DELETE" "/agents/$id" "" "true";
}
get_health_cmd() { ensure_curl_jq; echo "System Health..." >&2; _call_api "GET" "/system/health"; }
get_capabilities_cmd() { ensure_curl_jq; echo "System Capabilities..." >&2; _call_api "GET" "/system/capabilities"; }

COMMAND="$1"; if [ -z "$COMMAND" ]; then print_usage; exit 0; fi; shift;
case "$COMMAND" in
    create_agent) create_agent_cmd "$@";; list_agents) list_agents_cmd "$@";;
    get_agent) get_agent_cmd "$@";; update_agent) update_agent_cmd "$@";;
    delete_agent) delete_agent_cmd "$@";; get_health) get_health_cmd "$@";;
    get_capabilities) get_capabilities_cmd "$@";; help|--help|-h) print_usage;;
    *) echo "Error: Unknown command '$COMMAND'" >&2; print_usage >&2; exit 1;;
esac
exit $?
