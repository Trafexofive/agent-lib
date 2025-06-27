SHELL := /bin/bash

.PHONY: all build up down logs clean help

all: build

up: validate-env
	@echo "Starting Cadence Server..."
	@docker-compose up -d

down:
	@echo "Stopping Cadence Server..."
	@docker-compose down

build:
	@echo "Building Cadence Server image..."
	@docker-compose build

logs:
	@docker-compose logs -f cadence-server

clean:
	@echo "Cleaning up Cadence environment..."
	@docker-compose down -v --remove-orphans
	@echo "Cleanup complete."

fclean:
	@echo "Force cleaning Cadence environment..."
	@docker-compose down -v --remove-orphans
	@docker volume prune -f
	@docker network prune -f
	@docker builder prune -af
	@docker system prune -af
	@echo "Force cleanup complete."

validate-env:
	@if [ ! -f .env ]; then \
		echo "INFO: .env file not found. Copying from .env.example..."; \
		cp .env.example .env; \
		echo "Please review and edit the .env file with your secrets."; \
	fi

help:
	@echo "Cadence Server Makefile"
	@echo "-------------------------"
	@echo "up      - Start the server in detached mode."
	@echo "down    - Stop the server."
	@echo "build   - Build the server Docker image."
	@echo "logs    - Follow the server logs."
	@echo "clean   - Stop and remove all containers, volumes, and networks."
	@echo "fclean  - Force clean all containers, volumes, networks, and prune unused Docker resources."
