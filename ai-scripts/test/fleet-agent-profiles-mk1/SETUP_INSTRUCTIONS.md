# Fleet Agent Profiles MK1 - Setup Instructions (v0.1.0)

This relic contains a collection of specialized agent profiles (`.yml` files) and their corresponding system prompts (`.md` files). These profiles are designed to extend the capabilities of a `standard-agent-MK1` base by providing tailored configurations for specific roles within the Chimera Ecosystem.

## Included Profiles & Prompts:

1.  **DocumentationAgentMK1**:
    *   Profile: `standard-agent-MK1/documentation-agent-MK1.yml`
    *   Prompt: `standard-agent-MK1/system-prompts/documentation-agent-prompt.md`
    *   Role: Generates and manages documentation.

2.  **RelicForgeSpecialistAgentMK1**:
    *   Profile: `standard-agent-MK1/relic-forge-specialist-MK1.yml`
    *   Prompt: `standard-agent-MK1/system-prompts/relic-forge-specialist-prompt.md`
    *   Role: Interacts with the Relic Forge Bay API to manage software relics.

## Prerequisites:

-   An existing agent configuration system (e.g., where `standard-agent-MK1` is defined).
-   The `core.tools.yml` and `relic_forge_tools.yml` (for the RelicForgeSpecialist) must be discoverable by the agent loading mechanism, typically in a shared `tools/` directory relative to where the agent profiles are placed or as specified in the `import` section of the agent YAMLs.

## 1. Materialize Project

Using `relic_materializer.py`:
```bash
python relic_materializer.py path/to/this_plan.json --output-dir ./agent_configs --force
cd ./agent_configs/fleet-agent-profiles-mk1
```
This will create the `fleet-agent-profiles-mk1/` directory containing the `standard-agent-MK1/` subdirectory structure with the profiles and system prompts.

## 2. Integration with Agent System

1.  **Copy or Link:** Copy the materialized `standard-agent-MK1` directory (containing the new agent profiles and their `system-prompts` subdirectory) into your main agent configuration directory. For example, if your agents are in `main_config_dir/agents/`, you might copy it to `main_config_dir/agents/standard-agent-MK1/` (if you want to keep them grouped) or merge the contents appropriately.

2.  **Path Adjustments (If Necessary):
    *   The `system_prompt` paths within the agent `.yml` files are relative (e.g., `system-prompts/documentation-agent-prompt.md`). Ensure these paths correctly resolve from the final location of the agent `.yml` files.
    *   The `import.tools` paths (e.g., `tools/core.tools.yml`) are also relative. They assume a structure like:
        ```
        your_agent_base_config_dir/
        ├── agents/
        │   └── standard-agent-MK1/
        │       ├── documentation-agent-MK1.yml
        │       ├── relic-forge-specialist-MK1.yml
        │       └── system-prompts/
        │           ├── documentation-agent-prompt.md
        │           └── relic-forge-specialist-prompt.md
        └── tools/
            ├── core.tools.yml
            └── relic_forge_tools.yml
        ```
    Adjust paths in the YAML files if your directory structure differs significantly. The key is that the `system_prompt` path is relative *to the agent YAML file itself*, and the `import.tools` path is also typically resolved relative to the agent YAML file's location or a predefined tools directory known to your agent loader.

3.  **Agent Loading:** Ensure your agent loading mechanism can discover and parse these new YAML agent profiles.

## 3. Tool Dependencies:

-   **DocumentationAgentMK1** relies on tools defined in `core.tools.yml` (e.g., `bash`).
-   **RelicForgeSpecialistAgentMK1** relies on `core.tools.yml` AND `relic_forge_tools.yml`.

Ensure these tool definition files are correctly placed and their associated scripts are executable and in the correct paths as specified within those tool YAMLs.

## 4. Environment Variables:

-   The specialized agents might have their own `environment` sections in their YAMLs (e.g., `AGENT_ALIAS`).
-   The `RelicForgeSpecialistAgentMK1` scripts (via `common_vars.sh` in `relic_forge_tools.yml`) will use the `RELIC_FORGE_BASE_URL` environment variable. If not set in the execution environment of this agent, it will default to `http://localhost:13080` (as per the `relic-forge-agent-tools` v0.1.1).

## 5. Usage:

Once integrated, you should be able to invoke these agents by their names (e.g., `DocumentationAgentMK1`, `RelicForgeSpecialistAgentMK1`) through your agent management system. They will use their specialized system prompts and toolsets to perform tasks according to their defined roles.
