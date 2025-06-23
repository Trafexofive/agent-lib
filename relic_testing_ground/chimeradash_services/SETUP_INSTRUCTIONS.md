# ChimeraDash Services Relic Setup

This plan sets up a backend API service for your ChimeraDash frontend application.

## Prerequisites

*   Docker and Docker Compose (or Docker with Compose plugin) installed.
*   Your ChimeraDash frontend application source code, ready to be built into static assets.

## Setup Steps

1.  **Place Your Frontend Code:**
    *   Copy your entire ChimeraDash frontend application source code (including `package.json`, `vite.config.ts`, `src/`, `public/`, etc.) into the `frontend_service_code/` directory created by this relic plan.

2.  **Verify Frontend Build Output:**
    *   Ensure your frontend's build command (e.g., `npm run build` or `vite build` as defined in your `package.json`) outputs the static distributable files into a subdirectory named `dist/` directly inside `frontend_service_code/`. The provided Nginx Dockerfile (`frontend_service_code/Dockerfile`) expects this `dist/` folder.

3.  **Configure Environment:**
    *   Copy the example environment file: `cp .env.example .env`
    *   Open the `.env` file and adjust variables if needed (e.g., `BACKEND_PORT`, `FRONTEND_PORT`). The defaults are usually fine for local development.
    *   **Important for Frontend:** Your frontend application, when running inside its Docker container, will connect to the backend using the URL `http://chimeradash-backend:8000`. This is configured via the `API_BASE_URL` environment variable for the `chimeradash-frontend` service in `docker-compose.yml`. Ensure your Vite frontend code (e.g., in `App.tsx`, `services/geminiService.ts`, or wherever API calls are made) is set up to use an environment variable like `VITE_API_BASE_URL` and that this variable gets its value from `API_BASE_URL` during the Docker build or runtime.
        *   Example: In your Vite app, you might use `const apiUrl = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8000';`
        *   The `docker-compose.yml` already sets `API_BASE_URL` for the frontend container. You might need to pass this into Vite's build process if it's a build-time variable. For runtime, ensure your app reads it. One way is to pass it during the Docker build step if Vite embeds it, or have Nginx inject it (more complex).
        *   Simplest: Modify your Vite app to read `window.API_BASE_URL` if you inject it via `index.html`, or ensure your `Dockerfile` for the frontend can set `VITE_API_BASE_URL` during `npm run build` using the `API_BASE_URL` from the `docker-compose.yml` environment section. A common approach for Vite is to define `VITE_` prefixed env vars in a `.env` file within the frontend code, and `docker-compose.yml` can also provide these.

4.  **Start Services:**
    *   Open your terminal in the root directory of this relic (where `Makefile` and `docker-compose.yml` are located).
    *   Run the command: `make up`
    *   This will build the Docker images (if not already built) and start the backend and frontend services.

5.  **Access Services:**
    *   **Frontend (ChimeraDash UI):** Open your browser and go to `http://localhost:3000` (or the `FRONTEND_PORT` you configured in `.env`).
    *   **Backend API:** The API is accessible at `http://localhost:8000` (or the `BACKEND_PORT` you configured). You can test its endpoints:
        *   Context: `http://localhost:8000/context`
        *   Config: `http://localhost:8000/api/v1/config`

6.  **Test API with Client Script:**
    *   Make the client script executable: `chmod +x scripts/chimeradash_services_client.sh`
    *   Run the script: `./scripts/chimeradash_services_client.sh`
    *   This script will interact with the backend API endpoints.

7.  **Development Guidelines:**
    *   Refer to `GUIDELINES.md` for details on the project structure, agent interaction protocols, and how to modify different components of this relic.

## Common Makefile Commands

*   `make up`: Start all services in detached mode.
*   `make down`: Stop and remove all services and networks.
*   `make logs service=chimeradash-backend`: View logs for the backend service.
*   `make logs service=chimeradash-frontend`: View logs for the frontend service.
*   `make build service=chimeradash-backend`: Rebuild the backend service image.
*   `make ps` or `make status`: Show the status of running services.

Enjoy your ChimeraDash Services Relic!