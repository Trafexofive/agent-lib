## StorageMK1 Relic Setup (v0.2.1)

1.  **Bootstrap (Optional but Recommended for First Time):**
    Run `./_generated_bootstrap.sh` to create initial directories (`logs`, `data`, `config`, `storage_mk1_data_root`) and a template `.env` file if one doesn't exist. Make it executable first: `chmod +x _generated_bootstrap.sh`.

2.  **Review and Customize `.env`:**
    *   Open the `.env` file.
    *   Verify `BACKEND_PORT_HOST` and `FRONTEND_PORT_HOST` do not conflict with other services on your host.
    *   Set `DOMAIN_NAME` if you are using a custom domain for access (default is `localhost`).
    *   Configure `CORS_ALLOWED_ORIGINS` for the backend. If left empty, it defaults to `http://<DOMAIN_NAME>:<FRONTEND_PORT_HOST>` and `http://localhost:<FRONTEND_PORT_HOST>`. Use `*` to allow all origins (less secure).
    *   **Crucially, ensure `STORAGE_ROOT_HOST_PATH` points to the directory on your host machine where you want files to be stored.** The default is `./storage_mk1_data_root`.
    *   `VITE_API_BASE_URL` is intended for the frontend service; ensure it correctly points to your backend.

3.  **Provide Frontend Code & Dockerfile (MANDATORY for UI):**
    *   This relic plan **does not generate frontend code**. You must provide your own frontend application code within the `frontend/` directory.
    *   Your `frontend/` directory **must contain a `Dockerfile`** capable of building and serving your frontend application (e.g., for a Vite app, it might run `npm run dev -- --host`, or build static assets and serve with Nginx). A placeholder `frontend/Dockerfile` is provided as a template.
    *   Ensure your frontend application is configured to use the `VITE_API_BASE_URL` environment variable (passed by Docker Compose from `.env`) to make API calls to the backend.

4.  **Start the Services:**
    Run `make up`. This will build the backend image and attempt to build and run your user-provided frontend image.

5.  **Access Services:**
    *   **Backend API:** Should be accessible at `http://<DOMAIN_NAME>:<BACKEND_PORT_HOST>` (e.g., `http://localhost:8001`).
    *   **Frontend UI:** If you provided the frontend code and it starts correctly, it should be accessible at `http://<DOMAIN_NAME>:<FRONTEND_PORT_HOST>` (e.g., `http://localhost:5173`).

6.  **Check Logs:**
    Use `make logs service=storagemk1-backend-api` or `make logs service=frontend` to check the status and troubleshoot any issues.

7.  **Interact with API (CLI):**
    Use the client script: `chmod +x scripts/storagemk1_client.sh && ./scripts/storagemk1_client.sh`.

8.  **Consult `GUIDELINES.md`:**
    For detailed API endpoint information, agent interaction profiles, and development checklists, refer to `GUIDELINES.md`.