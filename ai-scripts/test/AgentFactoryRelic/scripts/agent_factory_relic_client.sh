#!/bin/bash
#
# Proper Command-Line Client for AgentFactoryRelic
# Version: 1.0
#
# A robust, interactive client for managing and executing agents.
#

# --- Configuration and Colors ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# --- Environment Loading ---
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ENV_FILE_PROJECT_ROOT="$SCRIPT_DIR/../.env"

if [ -f "$ENV_FILE_PROJECT_ROOT" ]; then
    source "$ENV_FILE_PROJECT_ROOT"
fi

BACKEND_PORT_HOST="${BACKEND_PORT_HOST:-8686}"
BACKEND_BASE_URL="http://localhost:${BACKEND_PORT_HOST}"
AGENT_CONFIG_DIR="$SCRIPT_DIR/../agent_configs"


# --- Prerequisite Check ---
check_deps() {
    if ! command -v curl &> /dev/null; then
        echo -e "${RED}Error: 'curl' is not installed. Please install it to use this script.${NC}"
        exit 1
    fi
    if ! command -v jq &> /dev/null; then
        echo -e "${RED}Error: 'jq' is not installed. Please install it for JSON parsing.${NC}"
        exit 1
    fi
}

# --- Help/Usage Function ---
show_help() {
    echo -e "${BLUE}AgentFactoryRelic CLI Client${NC}"
    echo -e "A tool for interacting with the Agent Factory API."
    echo -e "--------------------------------------------------"
    echo -e "${YELLOW}USAGE:${NC}"
    echo -e "  $0 <command> [arguments]"
    echo
    echo -e "${YELLOW}COMMANDS:${NC}"
    echo -e "  ${GREEN}help, -h${NC}             Show this help message."
    echo -e "  ${GREEN}context${NC}               Fetch the relic's /context endpoint."
    echo -e "  ${GREEN}list, list-agents${NC}   List all available agents from the '${BLUE}agent_configs${NC}' directory."
    echo -e "  ${GREEN}show <agent_name>${NC}    Display the configuration for a specific agent."
    echo -e "  ${GREEN}exec <agent_name>${NC}    Execute an agent. Must pipe the JSON payload via stdin."
    echo
    echo -e "${YELLOW}EXECUTION EXAMPLE:${NC}"
    echo -e "  Create a JSON file for your request (e.g., ${BLUE}request.json${NC}):"
    echo -e "    ${YELLOW}{ \"inputs\": { \"user_request\": \"Create a hello world script in Python.\" } }${NC}"
    echo
    echo -e "  Then execute the agent by piping the file content:"
    echo -e "    ${GREEN}cat request.json | $0 exec code-generator${NC}"
    echo
    echo -e "  Or using a one-liner with 'echo':"
    echo -e "    ${GREEN}echo '{\"inputs\":{\"user_request\":\"...\"}}' | $0 exec code-generator${NC}"
    echo
}

# --- Command Functions ---

get_context() {
    echo -e "${BLUE}Querying Relic Context...${NC}"
    curl -s -X GET "${BACKEND_BASE_URL}/context" | jq .
}

list_agents() {
    echo -e "${BLUE}Available agents in '${AGENT_CONFIG_DIR}':${NC}"
    if [ ! -d "$AGENT_CONFIG_DIR" ]; then
        echo -e "${RED}Error: Agent config directory not found at '${AGENT_CONFIG_DIR}'${NC}"
        return 1
    fi
    # List files, remove .yaml extension
    ls -1 "$AGENT_CONFIG_DIR" | sed 's/\.yaml$//'
}

show_agent_config() {
    local agent_name="$1"
    if [ -z "$agent_name" ]; then
        echo -e "${RED}Error: Agent name is required.${NC}"
        echo -e "Usage: $0 show <agent_name>"
        return 1
    fi
    local config_file="${AGENT_CONFIG_DIR}/${agent_name}.yaml"
    if [ ! -f "$config_file" ]; then
        echo -e "${RED}Error: Agent config '${config_file}' not found.${NC}"
        return 1
    fi
    echo -e "${BLUE}Configuration for agent '${YELLOW}${agent_name}${BLUE}':${NC}"
    echo -e "--------------------------------------------------"
    # Use 'bat' for syntax highlighting if available, otherwise use 'cat'
    if command -v bat &> /dev/null; then
        bat --paging=never --language=yaml --style=plain "$config_file"
    else
        cat "$config_file"
    fi
    echo -e "--------------------------------------------------"
}

execute_agent() {
    local agent_name="$1"
    if [ -z "$agent_name" ]; then
        echo -e "${RED}Error: Agent name is required.${NC}"
        echo -e "Usage: cat payload.json | $0 exec <agent_name>"
        return 1
    fi

    # Check if data is being piped via stdin
    if [ -t 0 ]; then
        echo -e "${RED}Error: No JSON payload provided via stdin.${NC}"
        echo -e "Please pipe the payload to the command."
        echo -e "Example: ${GREEN}cat request.json | $0 exec ${agent_name}${NC}"
        return 1
    fi

    echo -e "${BLUE}Executing agent '${YELLOW}${agent_name}${BLUE}'...${NC}"
    curl -s -X POST "${BACKEND_BASE_URL}/api/v1/agents/${agent_name}/execute" \
         -H "Content-Type: application/json" \
         -d @- | jq .
    echo -e "${GREEN}Execution request sent. Check response above and 'data/' directory for outputs.${NC}"
}

# --- Main Logic ---

# First, check dependencies
check_deps

# If no command is given, show help
if [ -z "$1" ]; then
    show_help
    exit 0
fi

COMMAND="$1"
shift # Shift arguments so the functions don't see the command itself

case "$COMMAND" in
    help|--help|-h)
        show_help
        ;;
    context)
        get_context
        ;;
    list|list-agents)
        list_agents
        ;;
    show|show-agent)
        show_agent_config "$1"
        ;;
    exec|execute)
        execute_agent "$1"
        ;;
    *)
        echo -e "${RED}Error: Unknown command '$COMMAND'${NC}"
        show_help
        exit 1
        ;;
esac
