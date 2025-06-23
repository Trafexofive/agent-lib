# The Boys: Diabolical Roundtable Chat - Setup Instructions (v0.1.2)

This document guides you through setting up and running The Boys themed chat server.

## Prerequisites

- Docker & Docker Compose
- A modern web browser
- A terminal/shell (bash compatible for Makefile)

## 1. Project Materialization

If you received this as a Full Relic Plan JSON file, use a materializer script (like `relic_materializer.py`) to create the project structure:

```bash
python relic_materializer.py the-boys-chat-plan_v0.1.2.json --output-dir ./projects --force
cd ./projects/the-boys-roundtable-chat
```

This will create the `the-boys-roundtable-chat/` directory with all specified files.

## 2. Environment Configuration (`.env`)

The application uses a `.env` file for configuration. The `Makefile` helps manage this.

1.  Navigate into the project directory (`the-boys-roundtable-chat`).
2.  The `make up` command (see step 3) will automatically call `make validate-env`.
    - This checks for `.env`. If not found, it checks for `.env.example`.
    - If `.env.example` exists, it will prompt you to copy it to `.env`.

   Default content of `.env.example` (and initially `.env`):
   ```env
   APP_PORT=8077
   CHAT_TITLE="The Boys: Diabolical Roundtable (PRIVATE) v0.1.2"
   MAX_HISTORY=100
   LOG_LEVEL=INFO
   ```

## 3. Build and Run with Docker

1.  **Build the Docker image** (optional, `make up` will do this if needed):
    ```bash
    make build
    ```
2.  **Start the application**:
    ```bash
    make up
    ```
    The API should become accessible at `http://localhost:PORT` (e.g., `http://localhost:8077` by default, or as shown in the `make up` output).

## 4. Accessing the Chat

1.  Open your web browser and navigate to the URL printed by `make up` (e.g., `http://localhost:8077`).
2.  Enter a 'Supe Codename' and click "Join the Mayhem".

## 5. Managing the Server

-   **View logs**: `make logs`
-   **Stop the server**: `make down`
-   **Access container shell**: `make shell`
-   **Full clean-up**: `make clean` (Deletes image, containers, and volumes)

## 6. Troubleshooting

-   **SyntaxError in `app/main.py`**: Version 0.1.2 specifically addresses this by simplifying string concatenation. If it persists, double-check that the content of `app/main.py` in your materialized project exactly matches the one in this plan, especially around lines 50-56 and 86-92. Ensure no stray backslashes or unescaped quotes were introduced during materialization.
-   **Port Conflict**: Change `APP_PORT` in `.env` and run `make down && make up`.
-   **Other Issues**: Check `make logs` and ensure Docker is running.

Enjoy the diabolical roundtable!