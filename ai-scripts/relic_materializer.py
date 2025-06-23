#!/usr/bin/env python3
# relic_materializer.py
import json
import argparse
from pathlib import Path
import sys
import os

def materialize_project(plan_file_path_str: str, output_base_dir_str: str, force_overwrite: bool = False):
    """
    Reads a Full Relic Plan JSON file and materializes the project structure
    and files it describes.

    Args:
        plan_file_path_str (str): Path to the Full Relic Plan JSON file.
        output_base_dir_str (str): The base directory where the project's
                                   root folder (named after 'relic_name')
                                   will be created.
        force_overwrite (bool): If True, will overwrite existing project directory.
    """
    plan_file_path = Path(plan_file_path_str).resolve()
    output_base_dir = Path(output_base_dir_str).resolve()

    if not plan_file_path.is_file():
        print(f"Error: Plan file not found at '{plan_file_path}'", file=sys.stderr)
        sys.exit(1)

    try:
        with open(plan_file_path, 'r', encoding='utf-8') as f:
            plan_data = json.load(f)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in plan file '{plan_file_path}': {e}", file=sys.stderr)
        print("This usually means the file is empty, not JSON, or has a syntax error.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: Could not read plan file '{plan_file_path}': {e}", file=sys.stderr)
        sys.exit(1)

    relic_name = plan_data.get("relic_name")
    if not relic_name or not isinstance(relic_name, str) or not relic_name.strip():
        print("Error: 'relic_name' not found in the plan, is invalid, or empty.", file=sys.stderr)
        sys.exit(1)
    
    # Sanitize relic_name for directory creation (basic sanitization)
    safe_relic_name = "".join(c if c.isalnum() or c in ['_', '-'] else '_' for c in relic_name)
    if not safe_relic_name:
        print(f"Error: Sanitized relic_name '{relic_name}' resulted in an empty string. Cannot create project directory.", file=sys.stderr)
        sys.exit(1)

    artifacts = plan_data.get("artifacts")
    if artifacts is None: # Could be an empty list, which is valid
        print("Warning: 'artifacts' array not found in the plan or is null. No files to create.", file=sys.stderr)
        artifacts = []
    if not isinstance(artifacts, list):
        print("Error: 'artifacts' in the plan is not a list.", file=sys.stderr)
        sys.exit(1)

    project_root_dir = output_base_dir / safe_relic_name

    if project_root_dir.exists():
        if force_overwrite:
            print(f"Warning: Project directory '{project_root_dir}' already exists. Overwriting due to --force flag.", file=sys.stderr)
            # Consider removing existing contents or just overwriting files.
            # For simplicity, we'll just proceed and let file writes overwrite.
            # A more robust solution might shutil.rmtree(project_root_dir) first.
        else:
            print(f"Error: Project directory '{project_root_dir}' already exists. Use --force to overwrite.", file=sys.stderr)
            sys.exit(1)
            
    try:
        project_root_dir.mkdir(parents=True, exist_ok=True) # exist_ok=True because of potential --force
        print(f"Project root directory: '{project_root_dir}'")
    except Exception as e:
        print(f"Error: Could not create project root directory '{project_root_dir}': {e}", file=sys.stderr)
        sys.exit(1)

    print(f"\nMaterializing {len(artifacts)} artifact(s) for relic '{relic_name}' (as '{safe_relic_name}'):")

    for i, artifact in enumerate(artifacts):
        if not isinstance(artifact, dict):
            print(f"Warning: Artifact at index {i} is not a dictionary. Skipping.", file=sys.stderr)
            continue

        artifact_path_str = artifact.get("path")
        artifact_content = artifact.get("content") # Content is expected to be a string
        artifact_title = artifact.get("title", f"Artifact {i+1}")

        if artifact_path_str is None or not isinstance(artifact_path_str, str) or not artifact_path_str.strip():
            print(f"Warning: Skipping artifact '{artifact_title}' due to missing, invalid, or empty 'path'.", file=sys.stderr)
            continue
        
        if artifact_content is None: # Allow empty string content
             print(f"Warning: Artifact '{artifact_title}' (path: {artifact_path_str}) has null content. Will create an empty file.", file=sys.stderr)
             artifact_content = "" # Default to empty string if content is null
        elif not isinstance(artifact_content, str):
            print(f"Warning: Content for artifact '{artifact_title}' (path: {artifact_path_str}) is not a string. Skipping.", file=sys.stderr)
            continue


        # Security: Prevent path traversal and absolute paths from the plan.
        # Paths in the plan should always be relative to the project root.
        # Normalize and resolve relative to a dummy root, then check components.
        normalized_artifact_path = Path(os.path.normpath(artifact_path_str))
        
        if normalized_artifact_path.is_absolute() or ".." in normalized_artifact_path.parts:
            print(f"Error: Invalid or unsafe path in artifact '{artifact_title}': '{artifact_path_str}'. Contains '..' or is absolute. Skipping.", file=sys.stderr)
            continue
        
        full_file_path = project_root_dir / normalized_artifact_path

        try:
            # Ensure parent directories exist within the project_root_dir
            full_file_path.parent.mkdir(parents=True, exist_ok=True)

            with open(full_file_path, 'w', encoding='utf-8') as f:
                f.write(artifact_content)
            print(f"  Created: {normalized_artifact_path} (Title: {artifact_title})")

        except Exception as e:
            print(f"Error creating artifact '{normalized_artifact_path}' (Title: {artifact_title}): {e}", file=sys.stderr)
            # Decide if you want to stop on first error or continue

    # Save setup_instructions_markdown if present
    setup_md = plan_data.get("setup_instructions_markdown")
    if isinstance(setup_md, str) and setup_md.strip():
        try:
            setup_file_path = project_root_dir / "SETUP_INSTRUCTIONS.md"
            with open(setup_file_path, 'w', encoding='utf-8') as f:
                f.write(setup_md)
            print(f"  Created: SETUP_INSTRUCTIONS.md")
        except Exception as e:
            print(f"Error creating SETUP_INSTRUCTIONS.md: {e}", file=sys.stderr)

    # Save bootstrap_script_content if present
    bootstrap_sh = plan_data.get("bootstrap_script_content")
    if isinstance(bootstrap_sh, str) and bootstrap_sh.strip(): # Ensure it's not null or empty before creating
        try:
            bootstrap_file_path = project_root_dir / "_generated_bootstrap.sh"
            with open(bootstrap_file_path, 'w', encoding='utf-8') as f:
                f.write(bootstrap_sh)
            # Make it executable by owner/group/others if on Unix-like system
            if os.name == 'posix':
                os.chmod(bootstrap_file_path, 0o755)
            print(f"  Created: _generated_bootstrap.sh (executable on POSIX)")
        except Exception as e:
            print(f"Error creating _generated_bootstrap.sh: {e}", file=sys.stderr)


    print(f"\nProject '{relic_name}' (as '{safe_relic_name}') materialized successfully in '{project_root_dir}'.")

def main_cli():
    parser = argparse.ArgumentParser(
        description="Materialize a project from a Full Relic Plan JSON file.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "plan_file",
        help="Path to the Full Relic Plan JSON file (e.g., my_relic_plan.json)."
    )
    parser.add_argument(
        "--output-dir",
        "-o",
        default=".",
        help="Base directory where the project folder (named after 'relic_name' from the plan) will be created. Defaults to the current directory."
    )
    parser.add_argument(
        "--force",
        "-f",
        action="store_true",
        help="Force overwrite if the project directory already exists."
    )
    args = parser.parse_args()

    materialize_project(args.plan_file, args.output_dir, args.force)

if __name__ == "__main__":
    main_cli()
