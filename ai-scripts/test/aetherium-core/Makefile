# ======================================================================================
# MISCELLANEOUS
# ======================================================================================

RED     := \033[0;31m
GREEN   := \033[0;32m
YELLOW  := \033[1;33m
BLUE    := \033[0;34m
NC      := \033[0m

# ======================================================================================
# GENERAL CONFIGURATION
# ======================================================================================

SHELL := /bin/bash

COMPOSE_FILE ?= docker-compose.yml
COMPOSE := docker-compose -f $(COMPOSE_FILE)

# ======================================================================================
# DEFAULT TARGET & SELF-DOCUMENTATION
# ======================================================================================
.DEFAULT_GOAL := help

.PHONY: help up down logs ps build no-cache restart re config status clean fclean prune stop start ssh exec inspect

# ======================================================================================
# HELP & USAGE
# ======================================================================================

help:
	@echo "Usage: make [target] [service=SERVICE_NAME]"
	@echo ""
	@echo "Core Stack Management:"
	@echo "  up, start       - Start all services."
	@echo "  down, stop      - Stop and remove all services."
	@echo "  restart         - Restart all services."
	@echo "  re              - Rebuild and restart services."
	@echo "  rere            - Force rebuild (no cache) and restart."
	@echo ""
	@echo "Information & Debugging:"
	@echo "  logs [service=<name>] - Follow logs."
	@echo "  status, ps      - Show service status."
	@echo "  ssh service=<name> - Shell into a service."
	@echo ""
	@echo "Cleaning:"
	@echo "  clean           - Remove stopped containers."
	@echo "  fclean          - Clean and remove volumes."
	@echo "  prune           - Docker system prune."

# ======================================================================================
# CORE STACK MANAGEMENT
# ======================================================================================

up: ## Start all services in detached mode
	@echo "Igniting services..."
	@$(COMPOSE) up -d --remove-orphans

start: up

down: ## Stop and remove all services
	@echo "Shutting down services..."
	@$(COMPOSE) down --remove-orphans

stop: down

restart: down up

re: down build up

rere: down no-cache up

# ======================================================================================
# BUILDING IMAGES
# ======================================================================================

build:
	@echo "Building images..."
	@$(COMPOSE) build $(service)

no-cache:
	@echo "Force-building images (no cache)..."
	@$(COMPOSE) build --no-cache $(service)

# ======================================================================================
# INFORMATION & DEBUGGING
# ======================================================================================

status: 
	@$(COMPOSE) ps $(service)
ps: status

logs:
	@$(COMPOSE) logs -f --tail="100" $(service)

ssh:
	@$(COMPOSE) exec $(service) /bin/sh || $(COMPOSE) exec $(service) /bin/bash

# ======================================================================================
# CLEANING & PRUNING
# ======================================================================================

clean:
	@$(COMPOSE) down --remove-orphans --rmi 'none'

fclean:
	@$(COMPOSE) down --volumes --remove-orphans --rmi 'none'

prune:
	@docker system prune -af --volumes
