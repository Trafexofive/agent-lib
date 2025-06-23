SHELL := /bin/bash
PROJECT_NAME ?= universal-ai-gateway
DOCKER_IMAGE_NAME ?= $(PROJECT_NAME)
TAG ?= latest
APP_SERVICE_NAME ?= gateway_app

APP_PORT = $(shell grep ^APP_PORT= .env 2>/dev/null | cut -d '=' -f2 | tr -d '[:space:]' || echo 8011)

.PHONY: all build up down logs clean validate-env help

all: build

build:
	@echo "Building Docker image $(DOCKER_IMAGE_NAME):$(TAG)..."
	@docker build -t $(DOCKER_IMAGE_NAME):$(TAG) .

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
			echo ".env.example copied to .env. Please review/customize API keys and other settings."; \
		else \
			echo "Skipping .env creation. Please ensure environment variables are set (e.g., API keys)."; \
		fi; \
	else \
		echo "INFO: .env file found."; \
	fi

up: validate-env
	@echo "Starting $(PROJECT_NAME) via Docker Compose... App expected on http://localhost:$(APP_PORT)"
	@docker-compose up -d

down:
	@echo "Stopping $(PROJECT_NAME)..."
	@docker-compose down

logs:
	@echo "Showing logs for $(APP_SERVICE_NAME) service..."
	@docker-compose logs -f $(APP_SERVICE_NAME)

clean:
	@echo "Cleaning up $(PROJECT_NAME) Docker environment..."
	@docker-compose down -v --remove-orphans
	@(docker rmi $(DOCKER_IMAGE_NAME):$(TAG) 2>/dev/null || echo "Image $(DOCKER_IMAGE_NAME):$(TAG) not found or already removed.")
	@echo "Local build artifacts (pycache, etc.) are not removed by this target."
	@echo "Clean complete."

help:
	@echo "Available commands for $(PROJECT_NAME):"
	@echo "  make build         - Build the Docker image"
	@echo "  make validate-env  - Check/create .env from .env.example"
	@echo "  make up            - Start the application (calls validate-env)"
	@echo "  make down          - Stop the application"
	@echo "  make logs          - View application logs"
	@echo "  make clean         - Clean Docker containers, volumes, and optionally the image."
	@echo "  make help          - Show this help message"
