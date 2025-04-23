#!/bin/bash

# Simple development script for agentttt backend and frontend

# --- Configuration ---
BACKEND_EXE="./agent-server"
FRONTEND_DIR="./dashboard"
FRONTEND_PORT="8000" # Port for the simple Python HTTP server
BACKEND_PID_FILE=".backend.pid"
FRONTEND_PID_FILE=".frontend.pid"

# Exit on any error
set -e

# --- Functions ---

# Function to build the backend
build_backend() {
    echo "--- Building Backend (make agent-server) ---"
    if make agent-server; then
        echo "--- Backend build successful ---"
    else
        echo "--- ERROR: Backend build failed! ---"
        exit 1
    fi
}

# Function to start the backend server
start_backend() {
    if [ -f "$BACKEND_PID_FILE" ]; then
        echo "Backend PID file found ($BACKEND_PID_FILE). Already running or needs cleanup?"
        echo "Run '$0 stop' first if needed."
        # Optionally: Check if PID is actually running: kill -0 $(cat $BACKEND_PID_FILE) 2>/dev/null
        return 1 # Indicate potential issue
    fi

    if [ ! -f "$BACKEND_EXE" ]; then
        echo "Backend executable '$BACKEND_EXE' not found. Building first..."
        build_backend
    fi

    # Source environment variables from .env file
    if [ -f ".env" ]; then
        echo "Loading environment variables from .env"
        export $(grep -v '^#' .env | xargs)
    else
        echo "Warning: .env file not found."
    fi

    # Check specifically for GEMINI_API_KEY
    if [ -z "$GEMINI_API_KEY" ] || [[ "$GEMINI_API_KEY" == "YOUR_API_KEY_HERE" ]]; then
        echo "--- ERROR: GEMINI_API_KEY not set correctly in environment or .env file! ---"
        exit 1
    fi


    echo "--- Starting Backend Server ($BACKEND_EXE) ---"
    # Run in background, redirect stdout/stderr to files, store PID
    "$BACKEND_EXE" > backend.log 2>&1 &
    BACKEND_PID=$!
    echo $BACKEND_PID > "$BACKEND_PID_FILE"
    echo "Backend PID: $BACKEND_PID (Logs: backend.log)"
    sleep 1 # Give it a moment to start or fail
    if ! kill -0 $BACKEND_PID 2>/dev/null; then
         echo "--- ERROR: Backend failed to start. Check backend.log ---"
         rm -f "$BACKEND_PID_FILE"
         exit 1
    fi
}

# Function to start the frontend HTTP server
start_frontend() {
     if [ -f "$FRONTEND_PID_FILE" ]; then
        echo "Frontend PID file found ($FRONTEND_PID_FILE). Already running or needs cleanup?"
        echo "Run '$0 stop' first if needed."
        return 1
    fi

    if [ ! -d "$FRONTEND_DIR" ]; then
        echo "--- ERROR: Frontend directory '$FRONTEND_DIR' not found! ---"
        exit 1
    fi

    echo "--- Starting Frontend HTTP Server (Python http.server on port $FRONTEND_PORT) ---"
    # Run in background within the specified directory, redirect output, store PID
    python -m http.server "$FRONTEND_PORT" --directory "$FRONTEND_DIR" > frontend.log 2>&1 &
    FRONTEND_PID=$!
    echo $FRONTEND_PID > "$FRONTEND_PID_FILE"
    echo "Frontend PID: $FRONTEND_PID (Logs: frontend.log)"
    echo "Access Frontend at: http://localhost:$FRONTEND_PORT"
    sleep 1
     if ! kill -0 $FRONTEND_PID 2>/dev/null; then
         echo "--- ERROR: Frontend server failed to start. Check frontend.log ---"
         rm -f "$FRONTEND_PID_FILE"
         exit 1
    fi
}

# Function to stop running servers
stop_servers() {
    echo "--- Stopping Servers ---"
    if [ -f "$BACKEND_PID_FILE" ]; then
        PID=$(cat "$BACKEND_PID_FILE")
        echo "Stopping Backend (PID: $PID)..."
        # Check if process exists before killing
        if kill -0 $PID 2>/dev/null; then
            kill $PID
            sleep 1 # Give it time to shut down
            # Force kill if still running (optional)
            # if kill -0 $PID 2>/dev/null; then kill -9 $PID; fi
        else
            echo "Backend process $PID not found."
        fi
        rm -f "$BACKEND_PID_FILE"
    else
        echo "Backend PID file not found. Server might not be running."
    fi

    if [ -f "$FRONTEND_PID_FILE" ]; then
        PID=$(cat "$FRONTEND_PID_FILE")
        echo "Stopping Frontend (PID: $PID)..."
        if kill -0 $PID 2>/dev/null; then
            kill $PID
        else
            echo "Frontend process $PID not found."
        fi
        rm -f "$FRONTEND_PID_FILE"
    else
        echo "Frontend PID file not found. Server might not be running."
    fi
    echo "--- Stop sequence complete ---"
}

# Function to display logs
show_logs() {
    echo "--- Tailing Logs (Press Ctrl+C to stop) ---"
    # Tail both logs concurrently
    tail -f backend.log frontend.log
}


# --- Main Script Logic ---

COMMAND=$1

case $COMMAND in
    start)
        start_backend || echo "Backend failed to start properly, skipping frontend."
        start_frontend || echo "Frontend failed to start properly."
        echo "--- Services started (check logs for details) ---"
        ;;
    stop)
        stop_servers
        ;;
    build)
        build_backend
        ;;
    restart)
        stop_servers
        echo # Add a newline for clarity
        start_backend || echo "Backend failed to start properly, skipping frontend."
        start_frontend || echo "Frontend failed to start properly."
        echo "--- Services restarted (check logs for details) ---"
        ;;
    logs)
        show_logs
        ;;
    *)
        echo "Usage: $0 {start|stop|build|restart|logs}"
        echo "  start   : Build (if needed) and start backend & frontend servers."
        echo "  stop    : Stop running servers."
        echo "  build   : Build the backend server."
        echo "  restart : Stop, then start servers."
        echo "  logs    : Tail the logs of both servers."
        exit 1
        ;;
esac

exit 0
