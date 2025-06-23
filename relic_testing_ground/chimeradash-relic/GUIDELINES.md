# Chimera Relic: chimeradash-relic - Agent Interaction & Development Guidelines

**Version:** `0.1.0`

This document provides essential guidelines for interacting with and modifying the **`chimeradash-relic`** software relic within the Chimera Ecosystem.

## 1. Agent Interaction Protocol

### 1.1 Key Components for Agent Awareness

*   **`chimeradash-relic-backend-api`:**
    *   **Purpose:** Core logic, data persistence, and configuration management for the ChimeraDash frontend.
    *   **Technology:** Python (FastAPI).
    *   **Key Interaction Point:** `/context` for capabilities, `/api/v1/config` for settings.
*   **`chimeradash-relic-frontend-ui`:**
    *   **Purpose:** Serves the user-provided ChimeraDash frontend application.
    *   **Technology:** Nginx serving static assets (built from user's Vite/React project).
    *   **Interaction:** Primarily user-facing. Agent interacts with backend.
*   **SQLite Database (Integrated into Backend):**
    *   **Path:** `backend_api_service/data/chimeradash_relic_data.db`
    *   **Purpose:** Persistence for some application settings.

### 1.2 Agent Context Endpoint (`/context`)

*   **URL:** `http://chimeradash-relic-backend-api:8000/context` (service name for inter-container comms)
*   **Method:** `GET`
*   **Response Example (Illustrative):**
    ```json
    {
      "relic_name": "chimeradash-relic",
      "version": "0.1.0",
      "description": "Backend API for ChimeraDash, managing configurations and providing data services to the frontend.",
      "system_prompt_fragment": "To use the ChimeraDash relic's API, an agent might query /api/v1/config for application settings such as 'dashboardTitle' or 'defaultTheme'. This API serves the ChimeraDash frontend.",
      "capabilities": [
        "Provides ChimeraDash system context via /context",
        "Manages ChimeraDash application configuration via /api/v1/config (GET, POST)",
        "Persists key settings in an SQLite database"
      ],
      "status_notes": "Operational. SQLite for data persistence is active."
    }
    ```

### 1.3 Tier 1 Modifiable Configuration (`/api/v1/config`)

*   **Read Configuration:**
    *   **URL:** `http://chimeradash-relic-backend-api:8000/api/v1/config`
    *   **Method:** `GET`
*   **Update Configuration:**
    *   **URL:** `http://chimeradash-relic-backend-api:8000/api/v1/config`
    *   **Method:** `POST`
    *   **Request Body Example:**
        ```json
        {
          "dashboardTitle": "New ChimeraDash Title from Agent",
          "defaultTheme": "Crimson Forge",
          "featureFlags": { "knowledgeGraphEnabled": false }
        }
        ```
    *   **Persistence:** Changes are saved to `backend_api_service/app_config.json` and relevant parts to the SQLite DB.

## 2. Feature Checklist & Development Roadmap

*   [X] Backend API (`chimeradash-relic-backend-api`)
    *   [X] `/context` endpoint.
    *   [X] `/api/v1/config` GET & POST endpoints for `dashboardTitle`, `defaultTheme`, `featureFlags`.
    *   [X] SQLite database for persisting `dashboardTitle`.
    *   [X] Serves user-provided ChimeraDash frontend via `chimeradash-relic-frontend-ui` service.
*   [X] Frontend Service Container (`chimeradash-relic-frontend-ui`)
    *   [X] Dockerfile to build & serve user's Vite/React app via Nginx.
    *   [X] `VITE_API_BASE_URL` environment variable for backend communication.
*   [X] DevOps Tooling (`Makefile`, `docker-compose.yml`, `.env`).

## 3. Modification Guidelines

### 3.1 Project Structure Overview

```
chimeradash-relic/
├── backend_api_service/         # Backend FastAPI
│   ├── Dockerfile
│   ├── main.py
│   ├── requirements.txt
│   ├── app_config.json
│   └── data/chimeradash_relic_data.db
├── chimeradash_frontend_code/   # USER PROVIDES THEIR ChimeraDash FRONTEND PROJECT HERE
│   ├── Dockerfile               # For Nginx to build & serve frontend
│   ├── nginx.conf
│   ├── package.json             # User's actual file
│   ├── vite.config.ts           # User's actual file
│   ├── tsconfig.json            # User's actual file (+ vite/client type)
│   ├── vite-env.d.ts            # For Vite env types
│   ├── index.html               # User's actual file
│   └── src/                     # User's actual frontend source
├── scripts/
│   └── chimeradash_relic_client.sh
├── .env
├── .gitignore
├── docker-compose.yml
├── Makefile
└── GUIDELINES.md
```

### 3.2 Modifying the Backend

*   Located in `backend_api_service/`.
*   Primary logic in `main.py`.
*   Add dependencies to `requirements.txt`, then `make build service=chimeradash-relic-backend-api`.

### 3.3 Integrating Your ChimeraDash Frontend

*   **ACTION REQUIRED:** Delete all placeholder files inside `chimeradash_frontend_code/` and copy your entire ChimeraDash frontend project (from `/home/mlamkadm/Downloads/chimeradash` or similar) into `chimeradash_frontend_code/`.
*   This includes your `package.json`, `vite.config.ts`, `tsconfig.json`, `index.html`, `src/` directory, etc.
*   The provided `chimeradash_frontend_code/Dockerfile` will attempt to build this project using `npm run build` (assuming this script exists in your `package.json` and outputs to a `dist/` folder).
*   Ensure your frontend application (e.g., in `services/geminiService.ts` or wherever API calls are made) uses `import.meta.env.VITE_API_BASE_URL` to get the backend URL. This is set by `docker-compose.yml` for the frontend container.
*   Ensure your frontend's `tsconfig.json` includes `"types": ["vite/client"]` in `compilerOptions` and that a `vite-env.d.ts` file exists in `chimeradash_frontend_code/` (or `chimeradash_frontend_code/src/`) to correctly type `import.meta.env`.

### 3.4 Environment Variables

*   Modify `.env` for ports. `VITE_API_BASE_URL_FOR_FRONTEND` is critical for frontend-backend communication within Docker.

---
*This document is tailored for the chimeradash-relic.*