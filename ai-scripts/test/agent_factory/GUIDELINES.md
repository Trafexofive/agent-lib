# GUIDELINES: Agent Factory Relic (v0.1.0)

This document provides essential guidelines for interacting with, developing, and extending the Agent Factory Relic. It is intended for both human operators (PRAETORIAN_CHIMERA) and other AI agents within the Chimera Ecosystem.

## 1. Agent Interaction Protocol

Interaction with this relic is primarily performed via its backend REST API. The API allows for the management of **Agent Definitions**, which are structured JSON objects that serve as blueprints for AI agents.

### Key Endpoints

- **`GET /context`**: Provides self-descriptive metadata about the relic for agent discovery.
- **`GET /api/v1/agents`**: Retrieves a list of all stored Agent Definitions.
- **`POST /api/v1/agents`**: Creates a new Agent Definition from a JSON body.
- **`GET /api/v1/agents/{agent_id}`**: Retrieves a specific Agent Definition by its ID.
- **`PUT /api/v1/agents/{agent_id}`**: Updates an existing Agent Definition.
- **`DELETE /api/v1/agents/{agent_id}`**: Deletes an Agent Definition.

### Agent Context Endpoint (`/context`)

- **Method**: `GET`
- **Path**: `/context`
- **Purpose**: To allow other agents to understand this relic's function and capabilities.
- **Example Response**:
  ```json
  {
    "relic_name": "agent_factory",
    "version": "0.1.0",
    "description": "A relic to create, manage, and store structured definitions (blueprints) for AI agents. It serves as the foundational component for an agent manufacturing and deployment system.",
    "system_prompt_fragment": "To interact with the Agent Factory, you should use its REST API. You can list all agent blueprints with a GET to /api/v1/agents. To create a new agent blueprint, POST a valid agent JSON object to /api/v1/agents. The required schema for an agent object can be retrieved by examining an existing agent or the default template.",
    "capabilities": [
      "Manages a database of agent definitions.",
      "Provides CRUD API for agent blueprints.",
      "Serves a lightweight frontend for human interaction."
    ],
    "status_notes": [
      "Operational. Database backend is PostgreSQL."
    ]
  }
  ```

## 2. The Agent Definition JSON Object

This is the core artifact managed by the factory. It's a detailed blueprint for an agent.

```json
{
    "agent_profile": {
        "name": "CoderBot-9000",
        "role": "Senior Python Developer",
        "description": "An AI agent specialized in writing, debugging, and refactoring Python code, with a focus on web APIs and data processing scripts."
    },
    "agent_directives": {
        "goals": [
            "Write clean, efficient, and well-documented Python code.",
            "Follow PEP 8 standards strictly.",
            "Prioritize tasks based on project requirements."
        ],
        "constraints": [
            "Do not execute any code that modifies the host filesystem outside of the designated /app/workspace directory.",
            "Do not interact with external APIs unless explicitly listed in the tool manifest.",
            "All code must include type hints."
        ],
        "personality_template": "You are a helpful and meticulous software engineering assistant. Your tone is professional, encouraging, and slightly formal. When asked for code, provide it directly, enclosed in markdown code blocks. Explain your reasoning clearly but concisely."
    },
    "llm_config": {
        "provider": "ollama",
        "model_name": "llama3:8b-instruct-q5_K_M",
        "parameters": {
            "temperature": 0.2,
            "top_p": 0.9,
            "mirostat": 2
        },
        "api_base_url_env_var": "OLLAMA_API_BASE_URL"
    },
    "tool_manifest": [
        {
            "tool_id": "python_code_interpreter",
            "description": "Executes a given string of Python code in a sandboxed environment and returns the stdout, stderr, and result.",
            "input_schema": {
                "type": "object",
                "properties": {
                    "code": { "type": "string" }
                },
                "required": ["code"]
            }
        }
    ],
    "memory_config": {
        "short_term": {
            "provider": "in_memory",
            "max_history_tokens": 4096
        },
        "long_term": {
            "provider": "vector_db",
            "notes": "Configuration for long-term memory via a vector database would be specified here."
        }
    },
    "output_schema": {
        "type": "object",
        "properties": {
            "thought_process": { "type": "string", "description": "Your reasoning and step-by-step plan for fulfilling the request." },
            "final_answer": { "type": "string", "description": "The final, complete answer or generated artifact (e.g., code, text)." },
            "tool_calls": { 
                "type": "array", 
                "items": {
                    "type": "object",
                    "properties": {
                        "tool_id": {"type": "string"},
                        "arguments": {"type": "object"}
                    }
                }
            }
        },
        "required": ["thought_process", "final_answer"]
    }
}
```

## 3. Key Components & Directory Structure

- **`/docker-compose.yml`**: Orchestrates the backend, frontend, and database services.
- **`/.env`**: Configuration variables. **MUST** be reviewed and updated.
- **`/Makefile`**: Standard Chimera Makefile for easy stack management.
- **`/service_code_backend/`**: Contains the FastAPI application.
  - `main.py`: API endpoints.
  - `database.py`, `db_models.py`, `db_crud.py`: Database logic (SQLAlchemy).
  - `pydantic_models.py`: Data validation models.
  - `default_agent_template.json`: The blueprint for the first agent created.
  - `alembic/`: Database migration scripts.
- **`/service_code_frontend/`**: Contains the vanilla JS/HTML/CSS frontend.
  - `src/index.html`: The main page layout.
  - `src/js/app.js`: All frontend logic for interacting with the backend API.
- **`/scripts/`**: Client scripts for CLI interaction.

## 4. Feature & Development Checklist (v0.1 -> v0.2)

- [x] **Backend**: Create CRUD API for agent definitions.
- [x] **Backend**: Use PostgreSQL with SQLAlchemy for data persistence.
- [x] **Backend**: Implement `/context` endpoint.
- [x] **Database**: Set up Alembic for migrations.
- [x] **Frontend**: Display list of agent definitions.
- [x] **Frontend**: View/Edit agent definitions in a form/editor.
- [x] **Frontend**: Create new agent definitions.
- [ ] **Backend**: Create a new endpoint `/api/v1/agents/{agent_id}/invoke`.
- [ ] **Backend**: Implement logic to connect to an Ollama instance using `llm_config` from the agent definition.
- [ ] **Backend**: Develop a simple tool-calling loop based on the `tool_manifest`.
- [ ] **Frontend**: Add an 'Invoke' button to the UI to test agents.
- [ ] **Memory**: Implement basic short-term memory (conversation history) during invocation.
- [ ] **Security**: Add input sanitization and validation on the invocation endpoint.

## 5. Modification Guidelines

1.  **To add a new field to the agent definition**: 
    1.  Update `service_code_backend/pydantic_models.py` with the new field.
    2.  Update the `Agent` model in `service_code_backend/db_models.py` if necessary.
    3.  Create a new migration using `make exec service=agent-factory-backend-api args="alembic revision --autogenerate -m 'add new_field to agent'"`.
    4.  Apply the migration by restarting the service (`make re`).
    5.  Update the frontend form in `service_code_frontend/src/js/app.js` to include the new field.

2.  **To add a new API endpoint**:
    1.  Define the endpoint in `service_code_backend/main.py`.
    2.  Implement the corresponding business logic, likely involving `db_crud.py`.
    3.  Add frontend functionality in `app.js` to call the new endpoint.
