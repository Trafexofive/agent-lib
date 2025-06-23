.PHONY: help setup

help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  setup   - Creates a Python virtual environment and installs dependencies."

setup:
	@echo "Creating virtual environment in ./.venv ..."
	python3 -m venv .venv
	@echo "Installing dependencies from requirements.txt ..."
	./.venv/bin/pip install -r requirements.txt
	@echo "Setup complete. Run the tool with './harvester'"

