#!/bin/bash
# Client for Agent Factory Relic (v0.1.0)

# Load .env variables if it exists in the script's directory or parent
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ENV_FILE_PROJECT_ROOT="$SCRIPT_DIR/../.env"

if [ -f "$ENV_FILE_PROJECT_ROOT" ]; then
    source "$ENV_FILE_PROJECT_ROOT"
fi

BACKEND_PORT_HOST="${BACKEND_PORT_HOST:-8001}"
BACKEND_BASE_URL="http://localhost:${BACKEND_PORT_HOST}"

function get_context() {
    echo -e "\n### Querying Agent Factory Relic Context Endpoint ###"
    curl -s -X GET "${BACKEND_BASE_URL}/context" | jq .
}

function list_agents() {
    echo -e "\n### Listing All Agent Definitions ###"
    curl -s -X GET "${BACKEND_BASE_URL}/api/v1/agents" | jq .
}

function get_agent() {
    AGENT_ID=$1
    if [ -z "$AGENT_ID" ]; then
        echo "Usage: $0 get <agent_id>"
        return
    fi
    echo -e "\n### Getting Agent Definition for ID: $AGENT_ID ###"
    curl -s -X GET "${BACKEND_BASE_URL}/api/v1/agents/${AGENT_ID}" | jq .
}

function create_agent() {
    FILE_PATH=$1
    if [ -z "$FILE_PATH" ]; then
        echo "Usage: $0 create <path_to_agent.json>"
        return
    fi
    if [ ! -f "$FILE_PATH" ]; then
        echo "Error: File not found at $FILE_PATH"
        return
    fi
    echo -e "\n### Creating New Agent from $FILE_PATH ###"
    curl -s -X POST "${BACKEND_BASE_URL}/api/v1/agents" \
        -H "Content-Type: application/json" \
        --data-binary "@${FILE_PATH}" | jq .
}

# --- Main Logic ---
COMMAND=$1
shift

case "$COMMAND" in
    context)
        get_context
        ;;
    list)
        list_agents
        ;;
    get)
        get_agent "$@"
        ;;
    create)
        create_agent "$@"
        ;;
    *)
        echo "Usage: $0 {context|list|get|create} [options]"
        echo "Commands:"
        echo "  context                - Get the relic's /context info."
        echo "  list                   - List all agent definitions."
        echo "  get <agent_id>         - Get a specific agent by its ID."
        echo "  create <path_to.json>  - Create an agent from a JSON file."
        ;;
esac

