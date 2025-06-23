### AgentFactoryRelic Setup Instructions (v0.1.3)

This update provides a new, powerful command-line client for interacting with the relic.

1.  **Clean Previous State (Optional):** If you have a previous version running, take it down.
    ```bash
    make down
    ```

2.  **Run Bootstrap Script:** If this is a new setup, run the bootstrap script to create directories and the example agent config.
    ```bash
    bash bootstrap.sh
    ```

3.  **Make Client Executable:** Ensure the new client script has execute permissions.
    ```bash
    chmod +x scripts/agent_factory_relic_client.sh
    ```

4.  **Start the Stack:** Use the Makefile to start the backend API and Ollama services.
    ```bash
    make up
    ```

5.  **PULL LLM MODEL (CRITICAL STEP):** The Ollama service starts empty. You must pull a model. Run in a separate terminal:
    ```bash
    make exec service=ollama args="ollama pull mistral:7b-instruct-q4_K_M"
    ```

6.  **Using the New Client:** Interact with your relic using the `agent_factory_relic_client.sh` script.

    *   **Get Help:** See all available commands.
        ```bash
        ./scripts/agent_factory_relic_client.sh help
        ```

    *   **List Available Agents:**
        ```bash
        ./scripts/agent_factory_relic_client.sh list
        ```

    *   **Show an Agent's Configuration:**
        ```bash
        ./scripts/agent_factory_relic_client.sh show code-generator
        ```

    *   **Execute an Agent:**
        Create a file named `payload.json` with your request:
        ```json
        {
          "inputs": {
            "user_request": "Create a Python script that prints the first 15 Fibonacci numbers."
          }
        }
        ```
        Then, pipe this file into the client:
        ```bash
        cat payload.json | ./scripts/agent_factory_relic_client.sh exec code-generator
        ```
        Check the terminal response and the `data/` directory for the output file.
