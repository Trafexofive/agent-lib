# ChimeraDash Services Relic Setup

This plan sets up a backend API service and a placeholder frontend service for your ChimeraDash application.

## Prerequisites

*   Docker and Docker Compose (or Docker with Compose plugin) installed.
*   Node.js and npm (or yarn) installed on your local machine if you intend to modify or rebuild the actual ChimeraDash frontend locally before placing it in the relic structure.

## Setup Steps

1.  **Review Placeholder Frontend (Optional but Recommended):**
    *   The `frontend_service_code/` directory contains placeholder files (`package.json`, `index.html`, `vite.config.ts`, `src/main.tsx`, `src/App.tsx`) for a minimal Vite + React + TypeScript application. 
    *   These placeholders allow the Docker build for the frontend service to succeed out-of-the-box, serving a basic placeholder page.

2.  **Integrate Your Actual ChimeraDash Frontend:**
    *   **Replace** the entire contents of the `frontend_service_code/` directory with your actual ChimeraDash frontend application source code.
    *   Ensure your frontend project includes a `package.json` with a `build` script (e.g., `"build": "tsc && vite build"`).
    *   Your frontend's build process must output its static assets to a `dist/` subdirectory within `frontend_service_code/`. The `frontend_service_code/Dockerfile` (Nginx-based) expects this `dist/` folder.

3.  **Configure Environment:**
    *   Copy the example environment file: `cp .env.example .env`
    *   Open the `.env` file and adjust variables if needed (e.g., `BACKEND_PORT`, `FRONTEND_PORT`). The defaults are usually fine for local development.
    *   **Frontend API Connection:** Your frontend application (e.g., `services/geminiService.ts` or `App.tsx` in ChimeraDash) should use an environment variable like `import.meta.env.VITE_API_BASE_URL` to connect to the backend. The `docker-compose.yml` sets `VITE_API_BASE_URL=http://chimeradash-backend:8000` for the frontend service. Ensure your Vite build process correctly picks up this variable.

4.  **Start Services:**
    *   Open your terminal in the root directory of this relic (where `Makefile` and `docker-compose.yml` are located).
    *   Run the command: `make up`
    *   This will build the Docker images (if not already built) and start the backend and frontend services.

5.  **Access Services:**
    *   **Frontend (ChimeraDash UI):** Open your browser and go to `http://localhost:3000` (or the `FRONTEND_PORT` you configured in `.env`). If you only used the placeholder frontend, you'll see a basic placeholder page. If you integrated your ChimeraDash UI, you should see your application.
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
*   `make build service=chimeradash-frontend`: Rebuild the frontend service image (after making changes to your code in `frontend_service_code/`).
*   `make ps` or `make status`: Show the status of running services.

Enjoy your ChimeraDash Services Relic!