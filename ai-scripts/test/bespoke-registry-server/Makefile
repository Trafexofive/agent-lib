SHELL := /bin/bash
PROJECT_NAME ?= bespoke-registry-server
DOCKER_IMAGE_NAME ?= $(PROJECT_NAME)
TAG ?= latest
APP_SERVICE_NAME ?= app

# Read APP_PORT from .env, default if not found. Use '=' for deferred expansion.
APP_PORT = $(shell grep ^APP_PORT= .env 2>/dev/null | cut -d '=' -f2 | tr -d '[:space:]' || echo 8009)

.PHONY: all build up down logs clean validate-env help test lint format

all: build

build:
	@echo "Building Docker image $(DOCKER_IMAGE_NAME):$(TAG)..."
	docker build -t $(DOCKER_IMAGE_NAME):$(TAG) .

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
			echo ".env.example copied to .env. Please review/customize it, especially GEMINI_API_KEY."; \
		else \
			echo "Skipping .env creation. Please ensure environment variables are set (e.g., APP_PORT, GEMINI_API_KEY)."; \
		fi; \
	else \
		echo "INFO: .env file found."; \
	fi

up: validate-env
	@echo "Starting $(PROJECT_NAME) via Docker Compose... App expected on http://localhost:$(APP_PORT)"
	docker-compose up -d

down:
	@echo "Stopping $(PROJECT_NAME)..."
	docker-compose down

logs:
	@echo "Showing logs for $(APP_SERVICE_NAME) service..."
	docker-compose logs -f $(APP_SERVICE_NAME)

clean:
	@echo "Cleaning up $(PROJECT_NAME) Docker environment..."
	docker-compose down -v --remove-orphans
	@(docker rmi $(DOCKER_IMAGE_NAME):$(TAG) 2>/dev/null || echo "Image $(DOCKER_IMAGE_NAME):$(TAG) not found or already removed.")
	rm -rf ./app/persistent_storage/uploaded_assets/* ./app/persistent_storage/registry_db/*
	@echo "Local build artifacts (pycache, etc.) are not removed by this target."
	@echo "Clean complete."

# Placeholder for tests - implement actual tests in tests/
# test:
# 	@echo "Running tests..."
# 	docker-compose run --rm $(APP_SERVICE_NAME) pytest tests/

# Linting and formatting require dev dependencies in requirements.txt and tools installed in image or locally
# lint:
# 	@echo "Linting with flake8..."
# 	docker-compose run --rm $(APP_SERVICE_NAME) flake8 app/
# 
# format:
# 	@echo "Formatting with black..."
# 	docker-compose run --rm $(APP_SERVICE_NAME) black app/

help:
	@echo "Available commands for $(PROJECT_NAME):"
	@echo "  make build         - Build the Docker image"
	@echo "  make validate-env  - Check/create .env from .env.example"
	@echo "  make up            - Start the application (calls validate-env)"
	@echo "  make down          - Stop the application"
	@echo "  make logs          - View application logs"
	@echo "  make clean         - Clean Docker containers, volumes, and optionally the image. Removes stored assets and DB."
	@echo "  make help          - Show this help message"
	@echo "
Note: test, lint, format targets are placeholders. Implement them as needed."
