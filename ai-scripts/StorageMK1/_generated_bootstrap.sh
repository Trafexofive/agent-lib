#!/bin/bash
# Bootstrap script for StorageMK1

echo "Bootstrapping StorageMK1 relic environment..."

mkdir -p logs
mkdir -p data
mkdir -p config
mkdir -p storage_mk1_data_root
touch storage_mk1_data_root/.gitkeep

echo "StorageMK1 bootstrap complete. Ensure 'storage_mk1_data_root' is configured in .env for STORAGE_ROOT_HOST_PATH."