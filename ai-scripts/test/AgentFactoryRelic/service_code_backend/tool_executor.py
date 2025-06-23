from typing import Dict, Any
import os

# This is a placeholder for where you would register and implement tools.
# For a real application, you might use a class-based system with registration.

def tool_read_file(filepath: str) -> str:
    """Reads the content of a file from the 'data' directory."""
    secure_path = os.path.join('data', os.path.basename(filepath))
    print(f"Attempting to read from: {secure_path}")
    try:
        with open(secure_path, 'r') as f:
            content = f.read()
        return f"Successfully read {len(content)} characters from {filepath}."
    except FileNotFoundError:
        return f"Error: File not found at {filepath}."
    except Exception as e:
        return f"An unexpected error occurred while reading file: {e}"

def tool_write_file(filepath: str, content: str) -> str:
    """Writes content to a file in the 'data' directory."""
    # Basic security: prevent path traversal attacks (e.g., ../../etc/passwd)
    secure_path = os.path.join('data', os.path.basename(filepath))
    print(f"Attempting to write to: {secure_path}")
    try:
        os.makedirs(os.path.dirname(secure_path), exist_ok=True)
        with open(secure_path, 'w') as f:
            f.write(content)
        return f"Successfully wrote {len(content)} characters to {filepath}."
    except Exception as e:
        return f"An unexpected error occurred while writing file: {e}"


# A dictionary mapping tool names from the YAML to actual Python functions.
AVAILABLE_TOOLS = {
    "read_file": tool_read_file,
    "write_file": tool_write_file,
}

def execute_tool(name: str, arguments: Dict[str, Any]) -> Any:
    if name not in AVAILABLE_TOOLS:
        return {"error": f"Tool '{name}' is not defined."}

    tool_function = AVAILABLE_TOOLS[name]
    try:
        return tool_function(**arguments)
    except TypeError as e:
        return {"error": f"Invalid arguments for tool '{name}': {e}"}
    except Exception as e:
        return {"error": f"An unexpected error occurred during tool execution: {e}"}
