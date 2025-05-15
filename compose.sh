#!/bin/bash
# compose.sh - Helper script for Docker Compose (Backend Focused)

set -e # Exit immediately if a command exits with a non-zero status.

# Check if .env file exists and contains necessary variables
if [ ! -f ".env" ]; then
    echo "---"
    echo "Warning: .env file not found."
    echo "Please create a .env file with your GEMINI_API_KEY."
    echo "Optionally, you can also set AGENT_PROFILE_PATH."
    echo "Example .env content:"
    echo "GEMINI_API_KEY=YOUR_API_KEY_HERE"
    echo "# AGENT_PROFILE_PATH=config/agents/your_custom_agent.yml"
    echo "---"
    # Consider exiting if key is mandatory for most operations: exit 1
fi

# Check if GEMINI_API_KEY is set and not placeholder in .env
if grep -q "^GEMINI_API_KEY=" .env; then
    if grep -q "^GEMINI_API_KEY=YOUR_API_KEY_HERE" .env || ! grep -q "^GEMINI_API_KEY=.\+" .env; then
        echo "---"
        echo "Error: GEMINI_API_KEY is set to placeholder or is empty in the .env file."
        echo "Please update .env with your actual GEMINI_API_KEY."
        echo "---"
        exit 1
    fi
else
    echo "---"
    echo "Error: GEMINI_API_KEY is not found in the .env file."
    echo "Please add GEMINI_API_KEY=YOUR_API_KEY_HERE to your .env file."
    echo "---"
    exit 1
fi


COMMAND=$1
shift # Remove the first argument (the command)

usage() {
    echo "Usage: $0 <command> [options]"
    echo "Manages the 'backend' service."
    echo "Commands:"
    echo "  up        Start backend service in detached mode"
    echo "  down      Stop and remove backend service"
    echo "  build     Build or rebuild backend service"
    echo "  logs      Follow backend service logs"
    echo "  restart   Restart backend service"
    echo "  ps        List running services"
    echo "  exec      Execute a command in the backend service (e.g., $0 exec bash)"
    echo "            (Service name 'backend' is implied for exec if not provided)"
    exit 1
}

# Default service for exec if not specified
EXEC_SERVICE_DEFAULT="backend"

case $COMMAND in
    up)
        echo "Starting backend service..."
        docker-compose up -d "$@" # "$@" will pass any additional args like --build
        ;;
    down)
        echo "Stopping and removing backend service..."
        docker-compose down "$@"
        ;;
    build)
        echo "Building backend service..."
        docker-compose build "$@" # Can specify 'backend' or leave for all services in compose file
        ;;
    logs)
        echo "Following backend logs (Ctrl+C to stop)..."
        docker-compose logs -f backend "$@" # Explicitly backend
        ;;
    restart)
        echo "Restarting backend service..."
        docker-compose restart backend "$@" # Explicitly backend
        ;;
    ps)
        docker-compose ps "$@"
        ;;
    exec)
        EXEC_TARGET_SERVICE=${1:-$EXEC_SERVICE_DEFAULT}
        if [ "$1" == "$EXEC_SERVICE_DEFAULT" ] || [ -z "$1" ] ; then # if first arg is 'backend' or empty
            shift || true # remove 'backend' if it was the first arg, or do nothing if it was empty
        fi

        if [ -z "$1" ] && [ "$EXEC_TARGET_SERVICE" == "$EXEC_SERVICE_DEFAULT" ]; then # Check if command is missing for default service
             echo "Error: Missing command for 'exec $EXEC_SERVICE_DEFAULT'."
             echo "Usage: $0 exec [service_name, defaults to backend] <command_to_run_in_container>"
             echo "Example: $0 exec bash"
             echo "Example: $0 exec backend ls -la /app"
             exit 1
        fi
        echo "Executing in '$EXEC_TARGET_SERVICE': $@"
        docker-compose exec "$EXEC_TARGET_SERVICE" "$@"
        ;;
    *)
        usage
        ;;
esac

exit 0
