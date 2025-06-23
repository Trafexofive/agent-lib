# Specialized Tool Suite MK1 - Setup Instructions (v0.1.0)

This relic provides a collection of specialized tool modules and their corresponding scripts, designed to enhance agent capabilities beyond basic command execution.

## Tool Modules Included:

1.  **File System Tools (`config/tools/file_system_tools.yml`):**
    - `fs_read_file`, `fs_write_file`, `fs_append_file`, `fs_list_directory_detailed`, `fs_check_existence`, `fs_get_file_properties`, `fs_create_tar_archive`, `fs_extract_tar_archive`.
    - Scripts in: `scripts/file_system/`

2.  **Network Utility Tools (`config/tools/network_utility_tools.yml`):**
    - `net_ping_host`, `net_curl_url`, `net_dns_lookup`.
    - Scripts in: `scripts/network_utility/`

3.  **Data Processing Tools (`config/tools/data_processing_tools.yml`):**
    - `data_json_validate`, `data_json_query_jq`, `data_text_search_grep`.
    - Scripts in: `scripts/data_processing/`

4.  **Code Utility Tools (`config/tools/code_utility_tools.yml`):**
    - `code_count_lines`, `code_simple_lint_python`.
    - Scripts in: `scripts/code_utility/`

5.  **Knowledge Interaction Tools (`config/tools/knowledge_interaction_tools.yml`):**
    - `kb_store_text`, `kb_retrieve_similar` (Conceptual: scripts are stubs).
    - Scripts in: `scripts/knowledge_interaction/`

## Prerequisites:

-   An agent execution system that can load YAML tool definitions and execute Bash scripts.
-   Essential command-line utilities must be installed in the agent's execution environment: `bash`, `jq`, `cat`, `find`, `stat`, `tar`, `ping`, `curl`, `dig`, `grep`, `wc`, `awk`, `python3`.

## 1. Materialize Project

Using `relic_materializer.py`:
```bash
python relic_materializer.py path/to/this_tool_suite_plan.json --output-dir ./agent_tool_suite_collection --force
cd ./agent_tool_suite_collection/specialized-tool-suite-mk1
```
This creates the `specialized-tool-suite-mk1/` directory with `config/tools/` and `scripts/` subdirectories.

## 2. Integration with Agent System

1.  **Tool Definitions (`config/tools/`):**
    *   Copy the `config/tools/*.yml` files into your agent system's designated tool configuration directory.
    *   Agents can import these modules (e.g., an agent profile might have `import: tools: - "path/to/file_system_tools.yml"`).

2.  **Scripts (`scripts/`):**
    *   Copy the entire `scripts/` directory structure (e.g., `scripts/file_system/`, `scripts/network_utility/`, etc.) into a location accessible by your agent's tool execution mechanism.
    *   The `path` attribute in each tool definition (e.g., `../../scripts/file_system/read_content.sh`) is relative *from the location of the tool's YAML file*. Ensure this resolves correctly in your setup. For example, if your tool YAMLs are in `your_agent_config_base/config/tools/`, the scripts directory should be sibling to `your_agent_config_base/config/` (i.e., `your_agent_config_base/scripts/`).

3.  **Permissions:** Make all shell scripts executable:
    ```bash
    find ./scripts -type f -name '*.sh' -exec chmod +x {} \;
    ```
    (Run this from the root of the materialized `specialized-tool-suite-mk1` directory, or adjust path).

## 3. Usage:

-   Agents configured to import these tool modules can then call the tools by their defined `name` (e.g., `fs_read_file`, `net_ping_host`).
-   Parameters must be provided as a JSON string, as expected by the scripts.
-   Scripts are designed to output JSON strings to stdout, which the agent system should capture as the tool's result.

## 4. Script Notes & Customization:

-   **Error Handling:** Scripts include basic error handling and attempt to return JSON error messages.
-   **JSON Escaping:** Some scripts (like `fs_read_file`) have basic JSON string escaping. For complex or binary content, a Python-based script might be more robust for handling and returning data as proper JSON.
-   **Security:** As always, executing arbitrary scripts or commands derived from LLM output carries risk. Ensure your agent system has appropriate sandboxing or safeguards if parameters (especially for `bash` or file paths) can be influenced by untrusted sources.
-   **Knowledge Base Stubs:** The `knowledge_interaction` tools are conceptual. Their scripts are stubs and would need to be implemented to connect to an actual knowledge base or vector database system.
-   **Path Resolution:** Many scripts take file paths as parameters. The agent must resolve these paths to be meaningful within its execution environment or workspace.
