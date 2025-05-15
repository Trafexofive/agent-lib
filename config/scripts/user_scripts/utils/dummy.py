# config/scripts/user_scripts/utils/dummy.py
import sys
import json

def main():
    print("Dummy Python script executed successfully.", file=sys.stderr)
    
    if len(sys.argv) > 1:
        params_json_str = sys.argv[1]
        try:
            params = json.loads(params_json_str)
            print("Received parameters:", file=sys.stderr)
            print(json.dumps(params, indent=2), file=sys.stderr)
            
            # Example: Echo back a specific parameter if present
            if params and isinstance(params, dict) and "echo_this" in params:
                print(f"Echoing from dummy: {params['echo_this']}")
            else:
                print("No 'echo_this' parameter found in params.")
                
        except json.JSONDecodeError:
            print("Error: Could not decode parameters JSON.", file=sys.stderr)
            print("Received as argument:", params_json_str, file=sys.stderr)
            print("Error: Invalid JSON parameters received by dummy.py") # Output for agent
            sys.exit(1)
        except Exception as e:
            print(f"Error processing parameters in dummy.py: {e}", file=sys.stderr)
            print(f"Error: Exception during dummy.py param processing: {e}") # Output for agent
            sys.exit(1)
    else:
        print("No parameters received by dummy.py.", file=sys.stderr)
        print("Dummy script completed without parameters.") # Output for agent

if __name__ == "__main__":
    main()
```*To make this accessible to `generic_python_executor.py` (which expects scripts under `config/scripts/user_scripts/`), you might need to adjust your `config/agents/standard-profiles/standard-agent-MK1/standard-agent-MK1.yml` if it calls this, or ensure the `python_exec` tool in `core.tools.yml` uses paths relative to `config/scripts/user_scripts/` when `generic_python_executor.py` resolves them.*

The `python_exec` in `core.tools.yml` uses `../scripts/core/generic_python_executor.py`.
`generic_python_executor.py` has `ALLOWED_SCRIPT_BASE_DIR = os.path.join(os.path.dirname(__file__), "..", "user_scripts")` which resolves to `config/scripts/user_scripts/`.
So a `script_path` like `"utils/dummy.py"` in the agent's YAML for a `python_exec` call *should* correctly point to `config/scripts/user_scripts/utils/dummy.py`.

Create the directory structure:
`mkdir -p config/scripts/user_scripts/utils`
And place `dummy.py` inside `config/scripts/user_scripts/utils/`.

---

**File 7: `inc/Utils.hpp` (Ensure Declarations)**
*Purpose: Confirm `logMessage` and `executeCommand` (if globally used) are declared.*

```cpp
#pragma once

#include <string>
#include <json/json.h> // For Json::Value used by executeScriptTool
#include <chrono>      // For logging timestamp components
#include <iomanip>     // For std::put_time
#include <iostream>    // For std::ostream

// Logging Enum
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

// Global/Utility Function Declarations
void logMessage(LogLevel level, const std::string &message, const std::string &details = "");

// Function to execute a simple command and get its output.
// Definition is in src/utils/global.cpp
int executeCommand(const std::string &command, std::string &output);

// Function to execute a script (inline or from file) with specified runtime and parameters.
// Definition is in src/utils/global.cpp
std::string executeScriptTool(const std::string& scriptPathOrContent,
                              const std::string& runtime,
                              const Json::Value& params, // Pass the full params object from ActionInfo
                              bool isContentInline);
