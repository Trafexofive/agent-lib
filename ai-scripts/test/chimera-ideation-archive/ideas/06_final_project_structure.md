# Architecture: Final Project Structure

The project structure is designed with strict separation of concerns, convention over configuration, and scalability in mind.

```
generic-automation-platform/
├── .env
├── docker-compose.yml
├── Makefile
├── README.md
│
├── data/                 # (PERSISTENT RUNTIME DATA)
│   └── knowledge_sources/
│
├── logs/                 # (RUNTIME LOGS)
│   └── gap_core.log
│
├── gap_core/             # (CORE ENGINE - The application itself)
│   ├── __init__.py
│   ├── Dockerfile
│   ├── main.py
│   ├── requirements.txt
│   │
│   ├── core/             # Heart of the platform's logic
│   │   ├── __init__.py
│   │   ├── agent_runner.py
│   │   ├── engine.py
│   │   ├── loader.py
│   │   └── tools.py
│   │
│   └── integrations/     # Built-in, platform-provided integrations
│       └── llm/
│
└── workspace/            # (USER CONFIGURATION - The user's primary domain)
    ├── agents/
    │   └── profiles/
    ├── orchestrations/
    ├── workflows/
    └── knowledge_bases.yaml
```

### Rationale

*   **`gap_core/` (Core Engine):** This is the application itself. Its name avoids collisions with Python standard libraries. The user should rarely need to modify this.
*   **`workspace/` (User Configuration):** This is the user's primary domain. It contains all their custom YAML definitions for agents, orchestrations, and workflows. This directory is cleanly separated from the core application code, allowing for easy updates to the engine without overwriting user configurations.
*   **`data/` (Runtime Data):** Contains stateful data generated *by* the platform, such as knowledge base sources. It's explicitly separated from code and configuration and should typically be managed as a Docker volume.
*   **Modularity:** This structure allows components to be managed as independent units. A new agent is just a new YAML file in `workspace/agents/profiles/`. A new core tool is a new class added to `gap_core/core/tools.py`. This makes the system highly extensible and maintainable.