.PHONY: build up down clean test lint format help

# Variables
COMPOSE_FILE = docker-compose.yml
IMAGE_NAME = yt-dlp-server
TAG = latest
APP_SERVICE_NAME = app

# Default target
all: build

# Build the Docker image
build:
	@echo "Building Docker image $(IMAGE_NAME):$(TAG)..."
	docker build -t $(IMAGE_NAME):$(TAG) .

# Start the services
up:
	@echo "Starting services using $(COMPOSE_FILE)..."
	@if [ ! -f .env ]; then \
		echo "INFO: .env file not found. Copying from .env.example..."; \
		cp .env.example .env; \
	fi
	docker-compose -f $(COMPOSE_FILE) up -d
	@echo "Services started. App should be available on port mapped to 8000."

# Stop and remove the services
down:
	@echo "Stopping and removing services..."
	docker-compose -f $(COMPOSE_FILE) down

# Clean up Docker images and containers
clean:
	@echo "Cleaning up Docker environment (containers, volumes, images)..."
	docker-compose -f $(COMPOSE_FILE) down --rmi all --volumes --remove-orphans
	@echo "Cleanup complete."

# Run tests
test:
	@echo "Running tests..."
	docker-compose -f $(COMPOSE_FILE) run --rm $(APP_SERVICE_NAME) pytest tests/

# Lint the codebase
lint:
	@echo "Linting codebase with Flake8..."
	docker-compose -f $(COMPOSE_FILE) run --rm $(APP_SERVICE_NAME) flake8 app/

# Format the codebase
format:
	@echo "Formatting codebase with Black..."
	docker-compose -f $(COMPOSE_FILE) run --rm $(APP_SERVICE_NAME) black app/

help:
	@echo "Available targets:"
	@echo "  build    - Build the Docker image"
	@echo "  up       - Start services (copies .env.example to .env if .env is missing)"
	@echo "  down     - Stop and remove services"
	@echo "  clean    - Perform a thorough cleanup of Docker artifacts"
	@echo "  test     - Run pytest tests (requires tests/ directory)"
	@echo "  lint     - Lint Python code in app/ with flake8"
	@echo "  format   - Format Python code in app/ with black"
	@echo "  help     - Show this help message"
