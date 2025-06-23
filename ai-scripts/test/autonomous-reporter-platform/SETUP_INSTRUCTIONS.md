# Autonomous Reporter Agent Platform - Setup Instructions (v0.1.2)

This v0.1.2 fixes a typo in `app/main.py` (`asnyc def` to `async def`). Core functionality remains focused on Agent/Report CRUD and Agent Import/Export, with information gathering and smart features still simulated.

## 1. Materialize Project

Use `relic_materializer.py` with the plan JSON:
```bash
python relic_materializer.py autonomous_reporter_platform_plan_v0.1.2.json --output-dir ./my_platform --force
cd ./my_platform/autonomous-reporter-platform
```

## 2. Environment Configuration (`.env`)

- `make validate-env` (or `make up`) will prompt to copy `.env.example` to `.env`.
- Review `.env`, especially `APP_PORT` (default `8010`) and `CORS_ALLOWED_ORIGINS`.

## 3. Create Host Directories for Persistent Storage

The `make up` command includes `mkdir -p` for these directories:
- `./app/platform_storage/agent_data`
- `./app/platform_storage/reports_db`

## 4. Build and Run with Docker

1.  Build: `make build`
2.  Start: `make up`
    Access at `http://localhost:PORT` (e.g., `http://localhost:8010`).

## 5. Interacting with the API

Use `curl` or a tool like Postman. See the `/api/v1/docs` endpoint for interactive API documentation once the server is running.

**Example: Create an agent manually**
```bash
curl -X POST -H "Content-Type: application/json" -d '\
{\
  "name": "ClimateMonitor_EU",\
  "description": "Tracks climate news in Europe.",\
  "keywords": ["climate action", "EU green deal", "carbon neutral"],\
  "monitoring_targets": ["https://ec.europa.eu/clima/news_en/rss.xml"],\
  "reporting_frequency": "PT6H"\
}' http://localhost:8010/api/v1/agents/ | jq
```

## 6. Key Implementation Notes & Next Steps

-   Information Gathering, Smart Agent Creation, Scheduling: These critical components remain **simulated** in `app/services_platform/`.
-   User Management: Still a placeholder.

## 7. Stopping and Cleaning Up

-   Stop: `make down`
-   Clean (removes data): `make clean`

This version primarily addresses the startup error from v0.1.1.