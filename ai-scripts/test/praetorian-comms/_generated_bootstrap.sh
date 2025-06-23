#!/bin/bash
# Bootstrap script for praetorian-comms v0.2.1

echo "Creating initial directory structure..."
mkdir -p backend_comms_server/data
mkdir -p backend_comms_server/api
mkdir -p backend_comms_server/core
mkdir -p backend_comms_server/files_storage
mkdir -p chat_client_frontend
mkdir -p admin_dashboard_frontend
mkdir -p scripts

echo "Bootstrap complete."
echo "USER ACTION REQUIRED:"
echo "1. Placeholder frontends have been created. Populate 'chat_client_frontend/' and 'admin_dashboard_frontend/' with your actual UI projects when ready."
echo "2. CRITICAL: Review and update '.env' with secure passwords and keys."
echo "3. Run 'make up' to start the services."