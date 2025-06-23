# Bespoke Registry Server (v0.1.0) - Setup Instructions

This server provides an API to act as a registry for various assets (files). It includes a **simulated** integration with the Gemini API for automatic tagging, quality rating, and name suggestion.

## Prerequisites

- Docker & Docker Compose (v1.28+ for Compose)
- `curl` and `jq` (optional, for `client.sh` and easier JSON viewing)
- A Bash-compatible shell

## 1. Materialize Project

If you have this plan as a JSON file (e.g., `bespoke_registry_server_plan.json`), use your `relic_materializer.py` script:

```bash
python relic_materializer.py bespoke_registry_server_plan.json --output-dir ./my_projects --force
cd ./my_projects/bespoke-registry-server
```
This command creates the `bespoke-registry-server/` directory with all the necessary files and subdirectories.

## 2. Environment Configuration (`.env`)

The application relies on an `.env` file for its configuration.

1.  The `make up` command (see below) will automatically try to copy `.env.example` to `.env` if `.env` is missing. You will be prompted.
2.  **CRITICAL**: After `.env` is created, you **MUST** edit it to provide your actual `GEMINI_API_KEY` if you intend to implement the real Gemini integration. For the current simulated version, this is not strictly necessary for the server to run, but it's good practice to set it or be aware of it.

   Example content of `.env.example` (and your initial `.env`):
   ```env
   APP_PORT=8009
   APP_NAME="Bespoke Registry Server"
   APP_VERSION="0.1.0"
   LOG_LEVEL=INFO
   REGISTRY_DB_URL_SQLITE_PATH=/app/persistent_storage/registry_db/registry.db
   UPLOADED_ASSETS_DIR_CONTAINER=/app/persistent_storage/uploaded_assets
   GEMINI_API_KEY="YOUR_GEMINI_API_KEY_HERE"
   CORS_ALLOWED_ORIGINS=http://localhost,http://localhost:8009,http://127.0.0.1:8009
   ```
   - `APP_PORT`: The port on your host machine that will map to the container's internal port (8009).
   - `CORS_ALLOWED_ORIGINS`: Adjust this if you are accessing the API from a frontend on a different origin.

## 3. Create Host Directories for Persistent Storage

Before running `make up` for the first time, it's good practice to create the directories on your host machine that will be mounted as volumes by Docker Compose. While Docker might create them, permissions can sometimes be an issue.

```bash
mkdir -p ./app/persistent_storage/uploaded_assets
mkdir -p ./app/persistent_storage/registry_db
```
These paths correspond to the `volumes` section in `docker-compose.yml`.

## 4. Build and Run with Docker

1.  **Build the Docker image**:
    ```bash
    make build
    ```
2.  **Start the application services**:
    ```bash
    make up
    ```
    The API server should now be running. The `make up` command will tell you the URL, typically `http://localhost:8009` (or your configured `APP_PORT`).

## 5. Interacting with the API (using `client.sh`)

Make the client script executable: `chmod +x client.sh`

1.  **Check Health & Capabilities**:
    ```bash
    ./client.sh health
    ./client.sh capabilities
    ```

2.  **Upload an Asset**:
    Create a dummy file, e.g., `touch my_test_file.txt` or `echo "Hello World" > my_document.txt`.
    ```bash
    # Usage: ./client.sh upload <file_path> [description]
    ./client.sh upload ./my_test_file.txt "This is a test asset for the registry"
    ```
    Note the `id` returned in the JSON response.

3.  **List Assets**:
    ```bash
    ./client.sh list
    ```

4.  **Get Specific Asset Metadata** (replace `<asset_id>` with an ID from the list/upload step):
    ```bash
    ./client.sh get <asset_id>
    ```

5.  **Download an Asset**:
    ```bash
    ./client.sh download <asset_id>
    ```
    This will save the file with its original name (or a generic one if the name isn't retrievable) in your current directory.

6.  **Delete an Asset**:
    ```bash
    ./client.sh delete <asset_id>
    ```

## 6. Implementing Real Gemini API Calls

This version of the server **SIMULATES** calls to the Gemini API.

To implement actual Gemini integration:

1.  **Install SDK**: Add `google-generativeai` to your `requirements.txt` (e.g., `google-generativeai==0.5.2`) and rebuild the Docker image (`make build`).
2.  **Configure API Key**: Ensure your `GEMINI_API_KEY` is correctly set in the `.env` file.
3.  **Modify `app/services/gemini_service.py`**: Update the `analyze_asset_with_gemini` function to:
    *   Initialize the Gemini client (`import google.generativeai as genai; genai.configure(api_key=settings.GEMINI_API_KEY)`).
    *   Choose an appropriate Gemini model (e.g., `gemini-pro` for text analysis, or a multimodal model if analyzing images/videos).
    *   Construct a suitable prompt using the `file_content_hash` (or actual `file_content` if you pass it, being mindful of size limits), `original_filename`, and `user_description`.
    *   Make the API call (e.g., `model.generate_content(prompt)`).
    *   Parse the response from Gemini to extract tags, a quality rating, and a suggested name. Handle potential errors from the API.
    *   Return a `GeminiAnalysisResult` object with the actual data.

## 7. Stopping and Cleaning Up

-   **Stop services**: `make down`
-   **Full cleanup** (removes containers, volumes including DB and uploaded assets, and the Docker image):
    ```bash
    make clean
    ```
    **Caution**: `make clean` is destructive and will remove all persisted data related to this server instance.

## Development Notes

-   Logs can be viewed with `make logs`.
-   The database file is `app/persistent_storage/registry_db/registry.db` on the host (mounted into the container).
-   Uploaded files are stored in `app/persistent_storage/uploaded_assets/` on the host.
