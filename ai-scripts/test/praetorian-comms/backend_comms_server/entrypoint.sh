#!/bin/sh

# This script is the container's entrypoint. It waits for the database to be ready
# and then runs the database initialization/migration logic before starting the main application.

set -e

# This is a simple wait loop. For a true production system, a more robust
# tool like 'wait-for-it.sh' would be used, but this is sufficient for a homelab relic.
echo "Entrypoint: Waiting for database to be ready..."

# The pg_isready check is now handled by the docker-compose healthcheck, 
# so a simple sleep is often enough to allow the service to be fully up after being healthy.
# If issues persist, a more complex loop could be added here.
sleep 5

echo "Entrypoint: Database is ready. Running initial setup/migrations..."

# Run the python script to create DB tables. Alembic would be used here in a more complex setup.
python initial_db_setup.py

echo "Entrypoint: Initial setup complete. Starting application..."

# Now, execute the command passed to this script (e.g., uvicorn, from the Dockerfile's CMD)
exec "$@"
