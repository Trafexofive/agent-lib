# Monitoring Agent Manager - Setup Instructions (v0.1.0)

This service provides a FastAPI backend to manage Monitoring Agent configurations. Data is stored in-memory.

## Prerequisites

- Docker & Docker Compose
- `curl` and `jq` (for `client.sh`)
- A terminal/shell (bash compatible)
- Your Lbro Universal `Makefile` placed in the project root directory.

## 1. Materialize Project

Using `relic_materializer.py`:
```bash
python relic_materializer.py path/to/this_plan.json --output-dir ./output --force
cd ./output/monitoring-agent-manager
```
Place your Lbro Universal `Makefile` in this `monitoring-agent-manager` directory.

## 2. Environment Configuration (`.env`)

This project uses an `.env` file for configuration. A `.env.example` is provided.

- If `.env` is missing, the `validate-env` Makefile target (called by `make up` or `make run-local`) will offer to copy `.env.example` to `.env`.
- Default `.env.example` content:
  ```env
  APP_NAME="Monitoring Agent Manager"
  APP_VERSION="0.1.0"
  APP_PORT=8020
  LOG_LEVEL=INFO
  CORS_ALLOWED_ORIGINS="*"
  ```
- The `APP_PORT` (default `8020`) in `.env` will be used by `docker-compose.yml` to map the host port to the container's internal port `8000`.
- The `client.sh` will also read `APP_PORT` from `.env` (or `.env.example`) to target the correct host port.

## 3. Build and Run with Docker

Ensure your Lbro Universal `Makefile` is in the project root.

1.  **Validate/Create `.env`** (done automatically by `make up` if `validate-env` is a prerequisite in your Universal Makefile, or run manually):
    ```bash
    make validate-env # Or similar if your Makefile has a different target name
    ```
2.  **Build the Docker image**:
    ```bash
    make build service=agent_manager_api # Assuming 'agent_manager_api' is the service name in docker-compose
    # Or, if your Makefile's build target builds all services by default:
    # make build 
    ```
3.  **Start the application**:
    ```bash
    make up service=agent_manager_api
    # Or, if 'up' starts all defined services:
    # make up
    ```
    The API should be accessible on the host at the port defined by `APP_PORT` (default `8020`).

## 4. Verify Installation

1.  **Check Docker logs** (adjust service name if needed):
    ```bash
    make logs service=agent_manager_api
    ```
2.  **Test health endpoint** (ensure `client.sh` is executable: `chmod +x client.sh`):
    ```bash
    ./client.sh get_health
    ```
    Expected `jq` formatted JSON output showing service health.

## 5. Using `client.sh` for API Interaction

Make `client.sh` executable: `chmod +x client.sh`

-   **Create an agent:**
    Create `example_agent.json`:
    ```json
    {
      "name": "Global Economic Monitor",
      "description": "Tracks detailed macroeconomic indicators.",
      "targets": [
        {
          "id": "imf-001",
          "description": "IMF DataMapper",
          "sourceType": "WEB_SEARCH",
          "sourceValue": "IMF DataMapper latest data",
          "informationToExtract": "GDP growth rates, inflation rates"
        }
      ],
      "reportFrequency": "Daily",
      "status": "active",
      "lastRunStatus": "pending"
    }
    ```
    Then run:
    ```bash
    ./client.sh create_agent example_agent.json
    ```
    Note the returned `id`.

-   **List agents:**
    ```bash
    ./client.sh list_agents
    ```

-   **Get a specific agent** (replace `<agent_id>`):
    ```bash
    ./client.sh get_agent <agent_id>
    ```

-   **Update an agent** (create `update_data.json` with fields to change, e.g., `{"status": "paused"}`):
    ```bash
    ./client.sh update_agent <agent_id> update_data.json
    ```

-   **Delete an agent**:
    ```bash
    ./client.sh delete_agent <agent_id>
    # Or to skip confirmation:
    # ./client.sh delete_agent <agent_id> --force
    ```

-   **Get capabilities:**
    ```bash
    ./client.sh get_capabilities
    ```

## 6. Stopping and Cleaning

(Adjust service name for your Universal Makefile if needed)
-   **Stop containers**: `make down service=agent_manager_api` or `make stop service=agent_manager_api`
-   **Full clean** (removes containers, image, and Docker volumes defined in `docker-compose.yml`):
    `make fclean service=agent_manager_api` (or equivalent in your Universal Makefile, be cautious as this deletes data if volumes are used for persistence).

This setup provides a basic, in-memory agent configuration manager. For persistent storage, you would need to integrate a database (e.g., SQLite, PostgreSQL) and update the CRUD operations and `docker-compose.yml` volume mounts accordingly.
