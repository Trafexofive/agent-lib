#!/bin/bash
# compose.sh - Helper script for Docker Compose

set -e # Exit immediately if a command exits with a non-zero status.

# Check if .env file exists
if [ ! -f ".env" ]; then
    echo "Error: .env file not found."
    echo "Please create a .env file with your GEMINI_API_KEY."
    echo "Example:"
    echo "GEMINI_API_KEY=YOUR_API_KEY_HERE"
    exit 1
fi

# Check if GEMINI_API_KEY is set in .env
if ! grep -q "^GEMINI_API_KEY=.\+" .env; then
    echo "Error: GEMINI_API_KEY is not set or is empty in the .env file."
    exit 1
fi


COMMAND=$1
shift # Remove the first argument (the command)

usage() {
    echo "Usage: $0 <command> [options]"
    echo "Commands:"
    echo "  up      Start services in detached mode"
    echo "  down    Stop and remove services"
    echo "  build   Build or rebuild services"
    echo "  logs    Follow service logs"
    echo "  restart Restart services"
    echo "  ps      List running services"
    echo "  exec    Execute a command in a service (e.g., ./compose.sh exec backend bash)"
    exit 1
}

case $COMMAND in
    up)
        echo "Starting services..."
        docker-compose up -d "$@"
        ;;
    down)
        echo "Stopping and removing services..."
        docker-compose down "$@"
        ;;
    build)
        echo "Building services..."
        docker-compose build "$@"
        ;;
    logs)
        echo "Following logs (Ctrl+C to stop)..."
        docker-compose logs -f "$@"
        ;;
    restart)
        echo "Restarting services..."
        docker-compose restart "$@"
        ;;
    ps)
        docker-compose ps "$@"
        ;;
    exec)
        if [ -z "$1" ]; then
            echo "Error: Missing service name for 'exec'."
            echo "Usage: ./compose.sh exec <service_name> <command>"
            exit 1
        fi
        docker-compose exec "$@"
        ;;
    *)
        usage
        ;;
esac

exit 0
