# Chimera Relic: chimeradash_services - Agent Interaction & Development Guidelines

**Version:** `0.1.0`
**Generated:** `System Generated Timestamp`

This document provides essential guidelines for interacting with and modifying the **`chimeradash_services`** software relic within the Chimera Ecosystem.

## 1. Agent Interaction Protocol

### 1.1 Key Components for Agent Awareness

The agent should be aware of the following primary services:

*   **`chimeradash-backend` (Backend API):**
    *   **Purpose:** Core logic, data persistence, and configuration management for the ChimeraDash frontend.
    *   **Technology:** Python (FastAPI).
    *   **Key Interaction Point:** `/context` endpoint for understanding relic capabilities and status. Agent-modifiable settings via `/api/v1/config`.
*   **`chimeradash-frontend` (Frontend UI):**
    *   **Purpose:** User interface for interacting with the ChimeraDash application.
    *   **Technology:** User-provided (e.g., React/Vite, served by Nginx). This relic plan includes placeholder frontend files to ensure the Docker build passes.
    *   **Interaction:** The agent typically interacts with the backend. Frontend settings might be locally stored or mirrored from backend config.
*   **SQLite Database (Integrated into Backend):**
    *   **Purpose:** Data persistence for backend settings or simple application data.
    *   **Technology:** SQLite.
    *   **Interaction:** Indirectly through the `chimeradash-backend` API.

### 1.2 Agent Context Endpoint (`/context`)

*   **URL:** `http://chimeradash-backend:8000/context` (service name and port within Docker network)
*   **Method:** `GET`
*   **Response:** JSON object detailing the relic's current state and capabilities.
    ```json
    {
      "name": "chimeradash_services",
      "version": "0.1.0",
      "description": "Backend services for the ChimeraDash application, providing API endpoints for frontend interaction and data management.",
      "system_prompt_fragment": "You are an AI assistant interacting with the ChimeraDash backend. Its primary role is to serve the ChimeraDash frontend application by managing its configuration and potentially other data services. It exposes a configuration API at /api/v1/config.",
      "capabilities": [
        "Provides system context via /context",
        "Manages backend configuration via /api/v1/config (GET, POST)",
        "Serves the ChimeraDash frontend application (user-provided)",
        "Stores basic settings in an SQLite database"
      ],
      "status_notes": "Operational. Backend is running and serving the user-provided frontend. Configuration is loaded."
    }
    ```

### 1.3 Tier 1 Modifiable Configuration (`/api/v1/config`)

The agent can read and update a subset of the backend's configuration.

*   **Read Configuration:**
    *   **URL:** `http://chimeradash-backend:8000/api/v1/config`
    *   **Method:** `GET`
    *   **Response:** JSON object of current Tier 1 settings.
        ```json
        {
          "sample_setting_1": "current_value_of_sample_setting_1",
          "feature_x_enabled": true
        }
        ```
*   **Update Configuration:**
    *   **URL:** `http://chimeradash-backend:8000/api/v1/config`
    *   **Method:** `POST`
    *   **Request Body:** JSON object with settings to update.
        ```json
        {
          "sample_setting_1": "new_value_for_sample_setting_1"
        }
        ```
    *   **Response:** Updated JSON configuration or success/error message.
    *   **Persistence:** Changes are saved to `backend_service_code/config.json` and potentially the SQLite DB, then reloaded by the backend.

### 1.4 Agent Task Examples

*   "Chimeradash Services, what is your current configuration for `sample_setting_1`?" (Agent fetches `/api/v1/config`)
*   "Chimeradash Services, update your `sample_setting_1` to 'anotherValue'." (Agent POSTs to `/api/v1/config`)
*   "Chimeradash Services, can you describe your capabilities?" (Agent fetches `/context`)

## 2. Feature Checklist & Development Roadmap

This relic currently supports:

*   [X] Backend API Service (`chimeradash-backend`)
    *   [X] `/context` endpoint
    *   [X] `/api/v1/config` GET endpoint
    *   [X] `/api/v1/config` POST endpoint (persists to `config.json` and example DB setting)
    *   [X] SQLite database for basic data storage (`data/relic_database.db`)
*   [X] Frontend Service (`chimeradash-frontend`)
    *   [X] Dockerized Nginx serving mechanism for user-provided or placeholder frontend static build.
    *   [X] Placeholder frontend files (`package.json`, `index.html`, `vite.config.ts`, `src/main.tsx`, `src/App.tsx`) to ensure successful initial build.
    *   [X] Environment variable `API_BASE_URL` (and `VITE_API_BASE_URL`) for backend communication from frontend.
*   [X] Standard DevOps Tooling
    *   [X] `docker-compose.yml` for multi-container orchestration
    *   [X] `Makefile` for common Docker operations
    *   [X] `.env.example` for environment configuration

**Future Enhancements (Illustrative):**
*   [ ] Implement user authentication for backend APIs.
*   [ ] Add more sophisticated data models and API endpoints based on ChimeraDash needs.
*   [ ] Integrate real-time communication (WebSockets) if ChimeraDash requires it.

## 3. Modification Guidelines

### 3.1 Project Structure Overview

```
chimeradash_services/
├── backend_service_code/        # Backend FastAPI application
│   ├── Dockerfile
│   ├── main.py
│   ├── requirements.txt
│   ├── config.json
│   └── data/
│       └── relic_database.db
├── frontend_service_code/       # User-provided frontend (or placeholders)
│   ├── Dockerfile               # For Nginx
│   ├── nginx.conf
│   ├── package.json             # (Placeholder or user's actual)
│   ├── vite.config.ts           # (Placeholder or user's actual)
│   ├── tsconfig.json            # (Placeholder or user's actual)
│   ├── index.html               # (Placeholder or user's actual)
│   └── src/
│       ├── main.tsx             # (Placeholder or user's actual)
│       └── App.tsx              # (Placeholder or user's actual)
│   └── dist/                    # (Expected build output)
├── scripts/
│   └── chimeradash_services_client.sh
├── .env.example
├── docker-compose.yml
├── Makefile
└── GUIDELINES.md
```

### 3.2 Modifying the Backend (`chimeradash-backend`)

*   **Location:** `backend_service_code/`
*   **Key Files:** `main.py`, `config.json`, `requirements.txt`.
*   To add new Python dependencies, update `requirements.txt` and rebuild the image (`make build service=chimeradash-backend`).
*   New API endpoints can be added in `main.py`.
*   Modify `config.json` directly for settings not exposed via the API, or to change defaults. Restart the service (`make restart service=chimeradash-backend`) for changes to take effect if they are only read at startup.

### 3.3 Modifying/Integrating the Frontend (`chimeradash-frontend`)

*   **Location:** `frontend_service_code/`
*   **Action Required by User:** *Replace the placeholder files* in this directory with your complete ChimeraDash frontend application source code.
*   **Placeholder Files:** This relic includes minimal `package.json`, `vite.config.ts`, `index.html`, and `src/main.tsx`, `src/App.tsx` to ensure the Docker build process (`npm install`, `npm run build`) for the frontend service can complete successfully out-of-the-box, serving a basic placeholder page.
*   **Build Process:** Your frontend's build process (e.g., `npm run build` or `vite build` as defined in your `package.json`) must output its static assets to a `dist/` subdirectory within `frontend_service_code/`. The provided `frontend_service_code/Dockerfile` (Nginx-based) expects this structure.
*   **API Connection:** The frontend application should be configured to make API calls to the backend. The `VITE_API_BASE_URL` environment variable is set to `http://chimeradash-backend:8000` for the `chimeradash-frontend` service in `docker-compose.yml`. Ensure your frontend code (e.g., Vite's `import.meta.env.VITE_API_BASE_URL`) uses this variable.

### 3.4 Environment Variables

*   Copy `.env.example` to `.env`.
*   Modify `.env` for your local setup. Key variables:
    *   `BACKEND_PORT`: Port for the backend service (e.g., `8000`).
    *   `FRONTEND_PORT`: Port for the frontend service (e.g., `3000`).

### 3.5 Data Persistence

*   **Backend SQLite DB:** Located at `backend_service_code/data/relic_database.db`. This path is volume-mounted via `backend_data` Docker volume in `docker-compose.yml` to persist data across container restarts.
*   **Frontend LocalStorage:** The ChimeraDash frontend likely uses browser `localStorage` for its own settings. This is managed by the user's browser and is separate from the backend persistence.

## 4. Troubleshooting

*   **`make logs service=chimeradash-backend`** or **`make logs service=chimeradash-frontend`**: Check logs for specific services.
*   **`docker-compose ps`** or **`make status`**: Check status of containers.
*   If frontend build fails (after replacing placeholders): Ensure your `package.json` and build scripts in `frontend_service_code/` are correct and that `npm run build` (or equivalent) successfully creates a `dist/` folder.

---
*This document is tailored for the chimeradash_services relic.*