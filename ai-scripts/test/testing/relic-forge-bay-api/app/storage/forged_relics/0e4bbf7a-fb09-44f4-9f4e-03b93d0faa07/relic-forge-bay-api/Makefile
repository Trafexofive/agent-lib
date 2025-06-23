# Lbro Universal Makefile (simplified for this project)
SHELL := /bin/bash
COMPOSE_FILE ?= docker-compose.yml
COMPOSE := docker-compose -f $(COMPOSE_FILE)

.PHONY: help build up down logs clean validate-env run-local test-health test-capabilities test-forge

help:
	@echo "Relic Forge Bay API Makefile"
	@echo "-----------------------------"
	@echo "Core Targets:"
	@echo "  build          - Build the Docker image for the API service."
	@echo "  up             - Start services in detached mode (calls validate-env)."
	@echo "  down           - Stop and remove services and networks."
	@echo "  logs           - Follow logs for the API service."
	@echo "  clean          - Stop services, remove containers, image, and data volumes."
	@echo "Development & Testing:"
	@echo "  validate-env   - Check/create .env from .env.example."
	@echo "  run-local      - Run the FastAPI app locally using Uvicorn (calls validate-env)."
	@echo "  test-health    - Test the /system/health endpoint using client.sh."
	@echo "  test-capabilities - Test the /system/capabilities endpoint using client.sh."
	@echo "  test-forge     - Create a dummy plan and test the /forge/relic endpoint."

validate-env:
	@if [ ! -f .env ]; then \
		echo "INFO: .env file not found."; \
		if [ ! -f .env.example ]; then \
			echo "ERROR: .env.example is also missing. Cannot proceed without configuration." >&2; \
			exit 1; \
		fi; \
		_USER_CHOICE=""; \
		read -r -p "Do you want to copy .env.example to .env? (Y/n) " _USER_CHOICE; \
		if [[ "$$_USER_CHOICE" == "Y" || "$$_USER_CHOICE" == "y" || "$$_USER_CHOICE" == "" ]]; then \
			cp .env.example .env; \
			echo ".env.example copied to .env. Please review/customize if needed."; \
		else \
			echo "Skipping .env creation. API may use default settings or fail if critical vars are missing."; \
		fi; \
	else \
		echo "INFO: .env file found."; \
	fi

build:
	@echo "Building Docker image for Relic Forge Bay API..."
	@$(COMPOSE) build relic-forge-bay-api

up: validate-env
	@echo "Starting Relic Forge Bay API via Docker Compose..."
	@$(COMPOSE) up -d --remove-orphans relic-forge-bay-api
	down:
	@echo "Stopping Relic Forge Bay API..."
	@$(COMPOSE) down --remove-orphans

logs:
	@echo "Showing logs for Relic Forge Bay API service..."
	@$(COMPOSE) logs -f relic-forge-bay-api

clean:
	@echo "Cleaning up Relic Forge Bay API Docker environment..."
	@$(COMPOSE) down -v --remove-orphans --rmi local
	@echo "Removing local storage directories (./app/storage)..."
	@rm -rf ./app/storage/forge_db
	@rm -rf ./app/storage/forged_relics
	@echo "Clean complete."

run-local: validate-env
	@echo "Starting Relic Forge Bay API locally with Uvicorn..."
	@echo "Ensure you have a Python virtual environment active with dependencies from requirements.txt"
	@FORGE_API_PORT=$$(grep '^FORGE_API_PORT=' .env 2>/dev/null | cut -d'=' -f2 | tr -d '[:space:]' || echo 8000); \
	uvicorn app.main:app --host 0.0.0.0 --port "$$FORGE_API_PORT" --reload

test-health:
	@./client.sh get_health

test-capabilities:
	@./client.sh get_capabilities

test-forge:
	@echo "Creating dummy_plan.json..."
	@echo '{
	  "relic_name": "DummyTestRelic",
	  "version": "0.0.1",
	  "artifacts": [
	    {"title": "README", "path": "README.md", "contentType": "text/markdown", "content": "# Dummy Test Relic\nThis is a test."},
	    {"title": "Data File", "path": "data/data.txt", "contentType": "text/plain", "content": "Sample data"}
	  ],
	  "setup_instructions_markdown": "# Setup\nThis is a dummy relic for testing."
	}' > dummy_plan.json
	@echo "Attempting to forge relic with dummy_plan.json..."
	@./client.sh forge_relic dummy_plan.json
	@echo "Test forge command sent. Check output above and list_relics for result."

