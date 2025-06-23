#!/bin/bash
echo "Bootstrapping Agent Factory Relic environment..."
mkdir -p service_code_backend/data service_code_backend/alembic service_code_frontend/src/css service_code_frontend/src/js scripts

# Create .gitkeep files to ensure directories are committed even if empty
touch service_code_backend/data/.gitkeep
touch service_code_frontend/src/css/.gitkeep
touch service_code_frontend/src/js/.gitkeep
touch scripts/.gitkeep

echo "Bootstrap complete. Directories created."