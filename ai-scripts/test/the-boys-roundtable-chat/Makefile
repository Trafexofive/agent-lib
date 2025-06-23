SHELL := /bin/bash
PROJECT_NAME := the-boys-roundtable-chat
APP_PORT = $(shell grep APP_PORT .env 2>/dev/null | cut -d '=' -f2 || echo 8077)

.PHONY: all build up down logs clean shell validate-env help

all: build

build:
	@echo "Building Docker image $(PROJECT_NAME):latest..."
	docker build -t $(PROJECT_NAME):latest .

validate-env:
	@if [ ! -f .env ]; then \
		echo "INFO: .env file not found."; \
		if [ ! -f .env.example ]; then \
			echo "Error: .env.example is also missing. Cannot proceed."; \
			exit 1; \
		fi; \
		_USER_CHOICE=""; \
		read -r -p "Do you want to copy .env.example to .env? (Y/n) " _USER_CHOICE; \
		if [[ "$$_USER_CHOICE" == "Y" || "$$_USER_CHOICE" == "y" || "$$_USER_CHOICE" == "" ]]; then \
			cp .env.example .env; \
			echo ".env.example copied to .env."; \
		else \
			echo "Skipping .env creation. Please ensure environment variables are set or defaults are used."; \
		fi; \
	else \
		echo "INFO: .env file found."; \
	fi

up: validate-env
	@echo "Starting $(PROJECT_NAME) via Docker Compose..."
	docker-compose up -d
	@echo "$(PROJECT_NAME) should be running on http://localhost:$(APP_PORT)"

down:
	@echo "Stopping $(PROJECT_NAME)..."
	docker-compose down

logs:
	@echo "Showing logs for $(PROJECT_NAME)..."
	docker-compose logs -f

clean:
	@echo "Cleaning up $(PROJECT_NAME)..."
	docker-compose down -v --remove-orphans
	(docker rmi $(PROJECT_NAME):latest 2>/dev/null || echo "Image not found or already removed, skipping.")
	rm -rf .pytest_cache .ruff_cache __pycache__ app/__pycache__ app/*/__pycache__ build dist *.egg-info
	@echo "Clean complete."

shell:
	@echo "Opening shell in running $(PROJECT_NAME) container..."
	docker-compose exec $(PROJECT_NAME) /bin/bash

help:
	@echo "Available commands:"
	@echo "  make build         - Build the Docker image"
	@echo "  make validate-env  - Check/create .env from .env.example"
	@echo "  make up            - Start the application containers (calls validate-env)"
	@echo "  make down          - Stop the application containers"
	@echo "  make logs          - View logs from running containers"
	@echo "  make clean         - Stop containers, remove volumes, image, and local caches"
	@echo "  make shell         - Open a bash shell inside the running application container"
	@echo "  make help          - Show this help message"
