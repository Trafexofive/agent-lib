#!/usr/bin/env python3
import sys
import json
import subprocess
import os

ALLOWED_SCRIPT_BASE_DIR = os.path.join(os.path.dirname(__file__), "..", "user_scripts") 

def main():
    if len(sys.argv) < 2:
        print("CRITICAL_ERROR: generic_python_executor.py requires at least one argument (the JSON parameters string).", file=sys.stderr)
        sys.exit(1)

    params_json_str = sys.argv[1]

    try:
        params = json.loads(params_json_str)
    except json.JSONDecodeError as e:
        print(f"CRITICAL_ERROR: Invalid JSON parameters string provided: {e}", file=sys.stderr)
        sys.exit(1)

    target_script_relative_path = params.get("script_path")
    script_specific_params_obj = params.get("script_params", {})

    if not target_script_relative_path or not isinstance(target_script_relative_path, str):
        print("CRITICAL_ERROR: 'script_path' string parameter is missing or invalid in JSON.", file=sys.stderr)
        sys.exit(1)

    abs_allowed_script_base_dir = os.path.abspath(ALLOWED_SCRIPT_BASE_DIR)
    prospective_script_path = os.path.join(abs_allowed_script_base_dir, target_script_relative_path)
    abs_target_script_path = os.path.normpath(os.path.abspath(prospective_script_path))

    if not abs_target_script_path.startswith(abs_allowed_script_base_dir + os.sep) and abs_target_script_path != abs_allowed_script_base_dir :
        print(f"SECURITY_ERROR: Target script path '{target_script_relative_path}' resolves outside of allowed base '{abs_allowed_script_base_dir}'. Resolved to '{abs_target_script_path}'. Execution denied.", file=sys.stderr)
        sys.exit(1)
        
    if not os.path.exists(abs_target_script_path) or not os.path.isfile(abs_target_script_path):
        print(f"CRITICAL_ERROR: Target script '{abs_target_script_path}' not found or not a file.", file=sys.stderr)
        sys.exit(1)

    script_params_json_str_for_target = json.dumps(script_specific_params_obj)
    command = ["python3", abs_target_script_path, script_params_json_str_for_target]

    print(f"Executing Python script: {' '.join(command)}", file=sys.stderr)

    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False,
            timeout=60
        )
        if result.stdout:
            print(result.stdout.strip())
        if result.stderr:
            print(f"Script STDERR for {target_script_relative_path}:\n{result.stderr.strip()}", file=sys.stderr)
        if result.returncode != 0:
            print(f"ERROR: Target script {target_script_relative_path} exited with status {result.returncode}", file=sys.stderr)
    except subprocess.TimeoutExpired:
        print(f"ERROR: Target script {target_script_relative_path} timed out after 60 seconds.", file=sys.stderr)
        print(f"Error: Script {target_script_relative_path} timed out.")
        sys.exit(1)
    except Exception as e:
        print(f"CRITICAL_ERROR: Failed to execute target Python script '{target_script_relative_path}': {e}", file=sys.stderr)
        print(f"Error: Could not execute script {target_script_relative_path} - {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
