# GUIDELINES.md - StorageMK1 v0.2.1

## 1. Agent Interaction Profile

This document provides guidelines for AI agents (and PRAETORIAN_CHIMERA) to interact with and modify the `StorageMK1` relic.

*   **Primary Function:** Provides basic Network Attached Storage (NAS) capabilities via an API, allowing for file and directory management on a designated storage volume. It is designed for use with a user-provided frontend.
*   **Interaction Method:** HTTP API requests to the `storagemk1-backend-api` service. The user is responsible for providing and running a separate frontend if UI interaction is desired.
*   **Key Endpoints for Agents:**
    *   `GET /context`: Retrieve relic metadata, capabilities, and status.
    *   `GET /api/v1/config`: Retrieve current backend application configuration.
    *   `POST /api/v1/config`: Update backend application configuration.
    *   `GET /api/v1/storage/browse?path=<path>`: List files and directories.
    *   `POST /api/v1/storage/upload?path=<path>`: Upload a file (multipart/form-data, field name `file`).
    *   `GET /api/v1/storage/download?filepath=<filepath>`: Download a file.
    *   `POST /api/v1/storage/mkdir`: Create a new directory (body: `{"path": "/new_folder"}`).
    *   `DELETE /api/v1/storage/delete`: Delete a file or directory (body: `{"path": "/item_to_delete"}`).
    *   `POST /api/v1/storage/move`: Move/rename a file or directory (body: `{"source_path": "...", "destination_path": "..."}`).

## 2. Key Components & Directory Structure

*   **`storagemk1-backend-api` (Python/FastAPI Service):**
    *   Location: `service_code_backend/`
    *   Core logic for API requests, file system operations.
    *   `main.py`: FastAPI application entry point, CORS configured via `.env`.
    *   `app_config.json`: Application configuration (storage path, upload limits etc.).
    *   `Dockerfile`: Builds the Python service.
*   **`frontend` (User-Provided Service):**
    *   Location: `frontend/` (user must populate this directory)
    *   The user is responsible for providing the frontend code and a `Dockerfile` in this directory.
    *   The `docker-compose.yml` includes a service definition for `frontend` which expects this structure.
    *   The frontend should use `VITE_API_BASE_URL` (or equivalent, as defined in `.env`) for API calls.
*   **Host Storage Volume:**
    *   Actual data stored on host at path specified by `STORAGE_ROOT_HOST_PATH` in `.env`.
    *   Mounted into backend container at `/storage_data`.
*   **DevOps & Configuration:**
    *   `docker-compose.yml`: Defines backend and frontend services, networks, volumes.
    *   `Makefile`: Standardized build, run, and manage commands.
    *   `.env`: Environment variables for ports, paths, domain, `VITE_API_BASE_URL`, `CORS_ALLOWED_ORIGINS`.
    *   `scripts/storagemk1_client.sh`: CLI script for basic API interaction.
    *   `_generated_bootstrap.sh`: Script to create initial directories and an example `.env` file.

## 3. Agent Context Endpoint (`/context`)

*   **Path:** `/context`
*   **Method:** `GET`
*   **Purpose:** Provides self-description for programmatic understanding by other Chimera agents.
*   **Expected Response Snippet (Illustrative):**
    ```json
    {
      "relic_name": "StorageMK1",
      "version": "0.2.1",
      "description": "A lightweight, modular mini-NAS relic providing API-driven file and directory management. Designed to be used with a user-provided frontend.",
      "system_prompt_fragment": "To manage files with StorageMK1, an agent can use endpoints under /api/v1/storage/. For example, to list files in '/documents', GET /api/v1/storage/browse?path=/documents. A frontend is expected to be provided by the user.",
      "capabilities": [
        "File and directory listing",
        "File upload and download",
        "Directory creation",
        "File and directory deletion",
        "File and directory move/rename",
        "Configurable storage backend via API (/api/v1/config)",
        "Self-description via /context endpoint",
        "Ready for user-provided Web Frontend integration"
      ],
      "status_notes": [
        "Operational. Storage root (container): /storage_data.",
        "Backend API ready. User to provide frontend code and Dockerfile in 'frontend/' directory.",
        "Frontend (if user-provided and running) expected at http://localhost:5173 (default, check .env)"
      ]
    }
    ```

## 4. Feature & Development Checklist (v0.2.1 Base)

**Backend (`storagemk1-backend-api`):**

*   [X] Basic FastAPI application structure.
*   [X] `GET /context` endpoint implementation.
*   [X] `GET /api/v1/config` endpoint (reads `app_config.json`).
*   [X] `POST /api/v1/config` endpoint (updates `app_config.json`).
*   [X] Storage API endpoints (`browse`, `upload`, `download`, `mkdir`, `delete`, `move`).
*   [X] `app_config.json` with settings, including `api_version` set to "0.2.1".
*   [X] `Dockerfile` for Python service.
*   [X] Robust error handling for file operations, including path validation.
*   [X] CORS middleware dynamically configured via `.env` (`CORS_ALLOWED_ORIGINS`, `DOMAIN_NAME`, `FRONTEND_PORT_HOST`).

**Frontend (`frontend` - User Provided):**

*   [ ] User to provide frontend application code (e.g., React, Vue, Svelte) in `frontend/`.
*   [ ] User to provide `frontend/Dockerfile` to build/serve their frontend application (e.g., using Vite dev server, or Nginx for static files).
*   [ ] Frontend should be configured to use the `VITE_API_BASE_URL` environment variable (set in `.env`) to communicate with the backend API.

**General & DevOps:**

*   [X] `docker-compose.yml` with backend and a placeholder frontend service expecting user code.
*   [X] `.env` file with configurations for ports, paths, `DOMAIN_NAME`, `VITE_API_BASE_URL`, `CORS_ALLOWED_ORIGINS`.
*   [X] Standard `Makefile`.
*   [X] `scripts/storagemk1_client.sh` for API interaction.
*   [X] This `GUIDELINES.md` document.
*   [X] `.gitignore` for common exclusions.
*   [X] `_generated_bootstrap.sh` for initial setup.

## 5. Modification Guidelines & Future Enhancements

*   **User Authentication & Authorization:** Implement user accounts and permissions.
*   **Metadata Database:** Add a database for richer metadata.
*   **Search Functionality:** API endpoints for searching files.
*   **File Versioning.**
*   **Sharing Capabilities.**
*   **Thumbnail Generation.**
*   **Chunked Uploads / Resumable Uploads.**
*   **Security Hardening:** Regularly review path validation, input sanitization, and dependency security.

Adhere to the Himothy Axioms and Chimera Prime Directives when modifying this relic.