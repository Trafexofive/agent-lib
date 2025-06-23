#!/bin/bash
# Relic: chimeradash_services Bootstrap
# This script is a placeholder for any initial setup.

echo "Bootstrapping chimeradash_services relic v0.1.0..."
echo "-----------------------------------------------"
echo "Ensuring Docker and Docker Compose are available..."

if ! command -v docker &> /dev/null
then
    echo "Docker could not be found. Please install Docker."
    exit 1
fi

if ! command -v docker-compose &> /dev/null
then
    # Fallback for newer 'docker compose' syntax
    if ! docker compose version &> /dev/null
    then 
        echo "Docker Compose could not be found. Please install Docker Compose (either standalone or as a Docker plugin)."
        exit 1
    fi
    echo "Using 'docker compose' (plugin syntax)."
else
    echo "Using 'docker-compose' (standalone syntax)."
fi

echo "Bootstrap check complete."
echo "To start the services, copy .env.example to .env, configure as needed, then run: make up"
echo "Refer to GUIDELINES.md and setup_instructions.md for more details."
