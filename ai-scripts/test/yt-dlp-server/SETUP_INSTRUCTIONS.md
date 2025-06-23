# YT-DLP Server (Relic Forger API) - Setup Instructions (v0.1.1)

This server currently implements a "Relic Forger" API, similar to the Relic Forge Bay. Despite its name and `yt-dlp` dependency, it **does not yet directly use `yt-dlp`** for video downloading or related functionalities. The `yt-dlp` dependency is present for future integration.

## Prerequisites

- Docker and Docker Compose installed (v1.28+ for Compose)
- Bash shell (for `client.sh`)
- `jq` (optional, for pretty-printing JSON in `client.sh`)

## 1. Materialize Project

If you have this plan as a JSON file (`yt-dlp-server_plan.json`), use your materializer script:
```bash
python relic_materializer.py yt-dlp-server_plan.json --output-dir ./projects --force
cd ./projects/yt-dlp-server
```
This creates the `yt-dlp-server/` directory with all files.

## 2. Environment Configuration (`.env`)

The `Makefile`'s `up` target will automatically copy `.env.example` to `.env` if `.env` does not exist.

Review the `.env` file. Key settings:
- `APP_PORT`: Port on the host machine that maps to the container's port 8000 (e.g., `APP_PORT=8000`).
- `ALLOWED_ORIGINS`: Comma-separated list of origins for CORS (e.g., `http://localhost:8000,http://yourfrontend.com`).
- `STORAGE_PATH`: Path inside the container for persistent data (default: `/app/storage`). This is mapped to `./storage` on your host by `docker-compose.yml`.

## 3. Create Storage Directories

Before the first run, create the host-side storage directory that will be mounted into the container:
```bash
mkdir -p ./storage/forge_db
mkdir -p ./storage/forged_relics
```
(The application will also attempt to create these inside the container path on startup if they don't exist, but it's good practice for host mounts.)

## 4. Build and Run with Docker

1.  **Build the Docker image**:
    ```bash
    make build
    ```
2.  **Start the application services**:
    ```bash
    make up
    ```
    The API server should now be running. Check the output for the host port (e.g., `http://localhost:8000`).

## 5. Verify Installation & API Interaction

Make the client script executable: `chmod +x client.sh`

1.  **Check Health**:
    ```bash
    ./client.sh health
    ```
2.  **Check Capabilities**:
    ```bash
    ./client.sh capabilities
    ```
3.  **Forge a Relic**:
    Create a `dummy_plan.json` file:
    ```json
    {
      "relic_name": "MyExampleApp",
      "version": "1.0.0",
      "artifacts": [
        {"title": "README", "path": "README.md", "contentType": "text/markdown", "content": "# My Example App\nThis is a test relic."},
        {"title": "Main Script", "path": "src/main.py", "contentType": "text/python", "content": "print('Hello from MyExampleApp!')"}
      ],
      "setup_instructions_markdown": "1. Unpack\n2. Run python src/main.py"
    }
    ```
    Then forge it:
    ```bash
    ./client.sh forge dummy_plan.json
    ```
    Note the `relic_id` from the response.

4.  **List Relics**:
    ```bash
    ./client.sh list
    ```

5.  **Download Relic** (replace `<relic_id>` with an actual ID):
    ```bash
    ./client.sh download <relic_id>
    tar -tvzf relic_<relic_id>.tar.gz
    ```

6.  **Delete Relic**:
    ```bash
    ./client.sh delete <relic_id>
    ```

## 6. Development Tasks (Linting, Formatting, Testing)

-   **Linting** (Flake8):
    ```bash
    make lint
    ```
-   **Formatting** (Black):
    ```bash
    make format
    ```
-   **Testing** (Pytest - requires test files in `./tests/`):
    ```bash
    # mkdir tests
    # # Add test files like tests/test_main.py
    # make test 
    ```

## 7. Stopping and Cleaning Up

-   **Stop services**: `make down`
-   **Full cleanup** (removes containers, volumes, images specified in compose): `make clean`

## Future `yt-dlp` Integration

To make this server a true `yt-dlp-server`, you would need to:
1.  Design API endpoints for submitting video URLs, choosing formats, etc.
2.  Implement corresponding FastAPI route handlers in `app/main.py`.
3.  Create or modify manager classes (e.g., a `YoutubeDLManager`) to encapsulate `yt-dlp` command execution logic (likely using `asyncio.create_subprocess_exec` for non-blocking calls to the `yt-dlp` CLI or its Python API if suitable for async).
4.  Update `client.sh` to interact with these new `yt-dlp` specific endpoints.
5.  Modify Pydantic models for `yt-dlp` request and response payloads.
