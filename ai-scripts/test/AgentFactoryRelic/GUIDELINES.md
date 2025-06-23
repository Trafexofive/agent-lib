# GUIDELINES: AgentFactoryRelic (v0.1.1)

This document provides essential guidelines for interacting with, modifying, and extending the `AgentFactoryRelic`.

## 1. Agent Interaction Contract

This relic is designed for programmatic interaction via its API. Human interaction is primarily for setup, monitoring, and creating new agent configurations.

### Agent Context Endpoint

- **Method:** `GET`
- **Path:** `/context`

This endpoint is the primary discovery mechanism. It returns a JSON object detailing the relic's name, version, capabilities, and a `system_prompt_fragment` that instructs other agents on how to use its core functionality.

### Core Functional Endpoint

- **Method:** `POST`
- **Path:** `/api/v1/agents/{agent_name}/execute`

This is the main endpoint for running an agent. 
- `{agent_name}` corresponds to the filename (without extension) in the `/agent_configs` directory.
- The request `body` must be a JSON object like: `{"inputs": {"key1": "value1", ...}}` where the keys match the `{{.key}}` variables in the agent's YAML `persona.prompt`.

## 2. Key Components & Directory Structure

- `Makefile`: Standard Chimera `Makefile` for managing the Docker Compose lifecycle.
- `docker-compose.yml`: Defines two core services: `agent-factory-relic-backend-api` and the `ollama` LLM service.
- `.env`: **Crucial configuration file.** `OLLAMA_API_BASE_URL` is now pre-configured to work within the Docker stack.
- `/agent_configs`: **The heart of the factory.** Place your `*.yaml` agent definitions here. The backend service mounts this directory.
- `/service_code_backend`: Contains all Python source code for the FastAPI application.
  - `main.py`: The FastAPI router and endpoint definitions.
  - `agent_runner.py`: Core logic for loading YAMLs, formatting prompts, and orchestrating the LLM/tool loop.
  - `ollama_client.py`: Handles all communication with the Ollama backend.
  - `tool_executor.py`: Maps tool names from YAML to executable Python functions. **This is a key file to modify when adding new tools.**
- `/data`: A directory mounted into the container, intended for tools like `write_file` to use as a sandboxed workspace.
- `volumes: ollama_data`: A named Docker volume to persist downloaded LLM models.

## 3. v0.1.1 Feature Checklist

**Backend / API:**
- [x] Load agent configurations from `/agent_configs/*.yaml`.
- [x] `/context` endpoint for agent self-discovery.
- [x] `/api/v1/agents/{agent_name}/execute` endpoint to trigger agent runs.
- [x] Parse `persona.prompt` and correctly substitute `inputs` from the request body.
- [x] Dynamically configure Ollama model based on YAML `model.source`.

**Tooling & LLM Integration:**
- [x] **Ollama service is now included and containerized within the stack.**
- [x] Connect to the internal Ollama service.
- [x] Send formatted prompt to Ollama and request JSON output.
- [x] Parse the `tool_call` object from the LLM response.
- [x] Execute the requested tool via the `tool_executor.py` mapping.
- [x] Implement a basic `write_file` tool that saves to the `/data` directory.

**Frontend:**
- [ ] No frontend in this plan. Backend is ready for future integration.

## 4. Modification Guidelines (How to Extend the Relic)

### How to Manage LLM Models

- The stack now includes Ollama. You must pull models into it before they can be used.
- **To pull a model:** `make exec service=ollama args="ollama pull <model_name>"` (e.g., `ollama pull mistral:7b-instruct-q4_K_M`)
- **To list downloaded models:** `make exec service=ollama args="ollama list"`

### How to Add a New Agent

1.  Create a new `your-agent-name.yaml` file inside the `/agent_configs` directory.
2.  Ensure the `model.source` in your YAML corresponds to a model you have pulled into the Ollama service.
3.  Restart the relic (`make re`) if you want to be sure, though it's not strictly necessary for config changes.
4.  You can now call `POST /api/v1/agents/your-agent-name/execute`.

### How to Add a New Tool

1.  **Define the function:** Open `service_code_backend/tool_executor.py`.
2.  Create a new Python function for your tool.
3.  **Register the function:** Add your new function to the `AVAILABLE_TOOLS` dictionary.
4.  **Update an Agent YAML:** Modify an agent's `.yaml` file to include your new tool in its `tools` list.
5.  **Restart the relic:** Run `make re` to apply the code changes.
