# Relic Forge Agent Tools - Setup Instructions (v0.1.1)

This relic packages a set of tools (a YAML definition and corresponding Bash scripts) for an agent to interact with a Relic Forge Bay API service. This version updates the default target URL in the scripts to `http://localhost:13080`.

## Prerequisites

- An agent system capable of loading tool definitions from YAML and executing Bash scripts.
- The target Relic Forge Bay API must be running and accessible from where the agent executes these scripts (typically on `http://localhost:13080` by default if using the companion `relic-forge-bay-api` project).
- `jq` and `curl` must be installed on the system where the agent executes these scripts.

## 1. Materialize Project

If you have this plan as a JSON file (e.g., `relic_forge_agent_tools_plan.json`), use your `relic_materializer.py` script (or equivalent) to create the files:

```bash
python relic_materializer.py relic_forge_agent_tools_plan.json --output-dir ./agent_tool_sets --force
cd ./agent_tool_sets/relic-forge-agent-tools
```
This creates the `relic-forge-agent-tools/` directory containing `tools/relic_forge_tools.yml` and the `scripts/relic_forge/` directory with all shell scripts.

## 2. Integrate with Agent

1.  **Tool Definition:** Copy or link `tools/relic_forge_tools.yml` into your agent's tool configuration directory or otherwise make it discoverable by your agent's tool loading mechanism.
2.  **Scripts:** Ensure the `scripts/` directory (containing `relic_forge/*.sh`) is placed such that the relative paths defined in `relic_forge_tools.yml` (e.g., `../scripts/relic_forge/forge_relic.sh`) correctly resolve from the location of the YML file. 
    *   For example, if `relic_forge_tools.yml` is in `agent_config/tools/`, then the scripts should be in `agent_config/scripts/relic_forge/`.
3.  **Permissions:** Make all the shell scripts executable:
    ```bash
    chmod +x scripts/relic_forge/*.sh
    ```

## 3. Environment Configuration (for Scripts)

The scripts in `scripts/relic_forge/` use an environment variable `RELIC_FORGE_BASE_URL` to determine the base URL of the Relic Forge Bay API. 

-   If `RELIC_FORGE_BASE_URL` is not set in the environment where the agent executes these scripts, they will default to `http://localhost:13080`.
-   To target a different API endpoint (e.g., if your Relic Forge Bay API is on a different host or port), set this environment variable in the agent's execution environment:
    ```bash
    export RELIC_FORGE_BASE_URL="http://your-forge-api-host:port"
    ```

## 4. Usage

Once integrated, the agent should be able to list these tools (e.g., `relic_forge_forge_relic`, `relic_forge_list_relics`, etc.) and execute them by providing the required parameters as specified in their `parameters_schema` within `relic_forge_tools.yml`.

**Example (conceptual agent interaction):**

Agent wants to list relics:
- Action: `relic_forge_list_relics`
- Params: `{}`

Agent wants to forge a relic:
- Action: `relic_forge_forge_relic`
- Params: `{"plan_json_string": "{\"relic_name\": \"MyNewTool\", \"version\": \"1.0.0\", \"artifacts\": []}"}`

Ensure the agent system correctly passes parameters as a single JSON string to the shell scripts.

## Dependencies of the Scripts

- `bash`: For running the scripts.
- `curl`: For making HTTP requests to the Relic Forge Bay API.
- `jq`: For parsing JSON parameters and formatting JSON output.

These must be installed on the system where the agent executes the tool scripts.
