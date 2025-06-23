#!/usr/bin/env python3
# relic_materializer_v3.0.py
import json
import argparse
import sys
import os
import shutil
from pathlib import Path

# --- Constants: Enhanced Operator Feedback ---
C_RED = "\033[91m"
C_GREEN = "\033[92m"
C_YELLOW = "\033[93m"
C_BLUE = "\033[94m"
C_BOLD = "\033[1m"
C_RESET = "\033[0m"

# --- Core Logic: The Unified Schema Dispatcher ---

def execute_plan(plan_file_path: Path, output_base_dir: Path, force: bool):
    """
    Reads a plan file, identifies its schema, and dispatches to the correct handler.
    This is the central engine of the materializer.
    """
    print(f"{C_BLUE}--- Initializing Relic Materializer v3.0 ---{C_RESET}")
    print(f"Reading plan from: '{plan_file_path}'")

    try:
        with open(plan_file_path, 'r', encoding='utf-8') as f:
            plan_data = json.load(f)
    except json.JSONDecodeError as e:
        _log_error(f"Invalid JSON in plan file: {e}")
    except Exception as e:
        _log_error(f"Could not read plan file: {e}")

    schema_version = plan_data.get("schema_version")
    if not schema_version:
        _log_error("Plan file is missing the required 'schema_version' field.")

    print(f"Detected schema version: {C_YELLOW}{schema_version}{C_RESET}")

    # The dispatch logic based on the schema version
    if schema_version == "rmp/v1.0":
        if force:
            _log_warn("--force flag is ignored for Relic Modification Plans.")
        _handle_relic_modification_plan(plan_data, output_base_dir)
    elif schema_version == "2.0":
        _handle_full_relic_plan(plan_data, output_base_dir, force)
    else:
        _log_error(f"Unsupported schema version: '{schema_version}'.")

# --- Handler: Full Relic Plan (The Rite of Annihilation and Rebirth) ---

def _handle_full_relic_plan(plan: dict, base_dir: Path, force: bool):
    """Orchestrates the creation of a new Relic from a FullRelicPlan."""
    relic_id = plan.get("metadata", {}).get("relic_id")
    if not relic_id:
        _log_error("FullRelicPlan is missing 'metadata.relic_id'.")

    project_root = base_dir.resolve() / relic_id
    print(f"Target project directory: '{project_root}'")

    # Validation-First Principle
    if project_root.exists():
        if force:
            # The Rite of Annihilation and Rebirth
            print(f"{C_YELLOW}{C_BOLD}WARNING:{C_RESET} Executing Rite of Annihilation and Rebirth.")
            print(f"Directory '{project_root}' will be completely destroyed and rebuilt due to --force flag.")
            try:
                shutil.rmtree(project_root)
            except Exception as e:
                _log_error(f"Failed to annihilate existing directory: {e}")
        else:
            _log_error(f"Project directory '{project_root}' already exists. Use --force to execute the Rite of Annihilation and Rebirth.")

    try:
        project_root.mkdir(parents=True, exist_ok=True)
    except Exception as e:
        _log_error(f"Could not create project root directory: {e}")

    print(f"\n{C_BLUE}--- Materializing Relic: {C_BOLD}{relic_id}{C_RESET}{C_BLUE} ---{C_RESET}")

    for group in plan.get("artifact_groups", []):
        group_name = group.get("group_name", "Untitled Group")
        print(f"\nProcessing artifact group: {C_YELLOW}{group_name}{C_RESET}")
        for artifact in group.get("artifacts", []):
            _create_artifact(artifact, project_root)

    _generate_env_example(plan.get("deployment_spec", {}), project_root)
    _create_setup_instructions(plan.get("setup_and_operation", {}), project_root)
    
    print(f"\n{C_GREEN}{C_BOLD}--- Relic '{relic_id}' materialized successfully. ---{C_RESET}")

# --- Handler: Relic Modification Plan (The Surgical Engine) ---

def _handle_relic_modification_plan(plan: dict, base_dir: Path):
    """Orchestrates the modification of an existing Relic."""
    relic_id = plan.get("target_relic_id")
    if not relic_id:
        _log_error("RelicModificationPlan is missing 'target_relic_id'.")

    project_root = base_dir.resolve() / relic_id
    comment = plan.get("comment", "No comment provided.")
    
    # Validation-First Principle
    if not project_root.is_dir():
        _log_error(f"Target Relic directory '{project_root}' does not exist. Cannot apply modifications.")

    print(f"\n{C_BLUE}--- Executing Modification Plan on Relic: {C_BOLD}{relic_id}{C_RESET}{C_BLUE} ---{C_RESET}")
    print(f"Plan objective: {comment}")
        
    operations = plan.get("operations", [])
    for i, op in enumerate(operations):
        op_name = op.get('op')
        print(f"\n{C_BOLD}Step {i+1}/{len(operations)}:{C_RESET} {C_YELLOW}{op_name}{C_RESET}")
        
        try:
            # The surgical dispatch to the correct operation primitive
            op_map = {
                "file.create": _op_file_create, "file.update": _op_file_update,
                "file.delete": _op_file_delete, "file.append": _op_file_append,
                "file.move": _op_file_move, "dir.create": _op_dir_create
            }
            if op_name in op_map:
                op_map[op_name](op, project_root)
            else:
                _log_warn(f"Unknown operation '{op_name}'. Skipping.")
        except Exception as e:
            _log_error(f"Operation failed: {e}\nHalting execution to prevent cascading errors.")
            
    print(f"\n{C_GREEN}{C_BOLD}--- Relic '{relic_id}' modified successfully. ---{C_RESET}")

# --- Operation Primitives & Artifact Helpers ---

def _create_artifact(artifact: dict, project_root: Path):
    path_str = artifact.get("path")
    if not path_str: return _log_warn("Skipping artifact with no path.")
    
    file_path = _validate_and_get_path(path_str, project_root)
    file_path.parent.mkdir(parents=True, exist_ok=True)
    file_path.write_text(artifact.get("content", ""), encoding='utf-8')
    print(f"  {C_GREEN}Created:{C_RESET} {path_str}")
    
    permissions_str = artifact.get("permissions")
    if permissions_str and os.name == 'posix':
        try:
            os.chmod(file_path, int(permissions_str, 8))
            print(f"    - Set permissions to {permissions_str}")
        except (ValueError, TypeError): _log_warn(f"Invalid permission format '{permissions_str}'.")

def _op_file_create(op: dict, root: Path):
    path = _validate_and_get_path(op['path'], root)
    if path.exists(): raise FileExistsError(f"Cannot create '{op['path']}'; path already exists.")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(op['content'], encoding='utf-8')
    print(f"  {C_GREEN}Success:{C_RESET} Created file '{op['path']}'")

def _op_file_update(op: dict, root: Path):
    path = _validate_and_get_path(op['path'], root)
    if not path.is_file(): raise FileNotFoundError(f"Cannot update '{op['path']}'; file does not exist.")
    path.write_text(op['content'], encoding='utf-8')
    print(f"  {C_GREEN}Success:{C_RESET} Updated file '{op['path']}'")

def _op_file_delete(op: dict, root: Path):
    path = _validate_and_get_path(op['path'], root)
    if not path.is_file(): raise FileNotFoundError(f"Cannot delete '{op['path']}'; file does not exist.")
    path.unlink()
    print(f"  {C_GREEN}Success:{C_RESET} Deleted file '{op['path']}'")

def _op_file_append(op: dict, root: Path):
    path = _validate_and_get_path(op['path'], root)
    if not path.is_file(): raise FileNotFoundError(f"Cannot append to '{op['path']}'; file does not exist.")
    with path.open('a', encoding='utf-8') as f: f.write(op['content'])
    print(f"  {C_GREEN}Success:{C_RESET} Appended to file '{op['path']}'")

def _op_file_move(op: dict, root: Path):
    from_path = _validate_and_get_path(op['from_path'], root)
    to_path = _validate_and_get_path(op['to_path'], root)
    if not from_path.exists(): raise FileNotFoundError(f"Cannot move '{op['from_path']}'; source does not exist.")
    if to_path.exists(): raise FileExistsError(f"Cannot move to '{op['to_path']}'; destination already exists.")
    to_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(from_path), str(to_path))
    print(f"  {C_GREEN}Success:{C_RESET} Moved '{op['from_path']}' -> '{op['to_path']}'")

def _op_dir_create(op: dict, root: Path):
    path = _validate_and_get_path(op['path'], root)
    if path.exists(): raise FileExistsError(f"Cannot create directory '{op['path']}'; path already exists.")
    path.mkdir(parents=True, exist_ok=False)
    print(f"  {C_GREEN}Success:{C_RESET} Created directory '{op['path']}'")

def _generate_env_example(spec: dict, root: Path):
    env_vars = spec.get("env_variables", [])
    if not env_vars: return
    content = [f"# .env.example for {root.name}\n"] + [f"# {v.get('description', '')}\n{v.get('name')}={v.get('default', '')}\n" for v in env_vars if v.get("name")]
    (root / ".env.example").write_text("\n".join(content), encoding='utf-8')
    print(f"\n{C_YELLOW}Generated:{C_RESET} .env.example")

def _create_setup_instructions(spec: dict, root: Path):
    if instructions := spec.get("setup_instructions_markdown"):
        (root / "SETUP_INSTRUCTIONS.md").write_text(instructions, encoding='utf-8')
        print(f"{C_YELLOW}Generated:{C_RESET} SETUP_INSTRUCTIONS.md")

# --- Utilities: Security & Logging ---

def _validate_and_get_path(path_str: str, project_root: Path) -> Path:
    if ".." in Path(path_str).parts:
        raise PermissionError(f"Path Traversal Violation: '..' is not allowed in path '{path_str}'.")
    
    full_path = (project_root / path_str).resolve()
    if project_root.resolve() not in full_path.parents and full_path != project_root.resolve():
        raise PermissionError(f"Path Traversal Violation: Attempted to access '{path_str}' outside of '{project_root}'.")
        
    return full_path

def _log_error(message: str):
    print(f"\n{C_RED}{C_BOLD}FATAL ERROR:{C_RESET}{C_RED} {message}{C_RESET}", file=sys.stderr)
    sys.exit(1)

def _log_warn(message: str):
    print(f"{C_YELLOW}{C_BOLD}WARNING:{C_RESET} {message}", file=sys.stderr)

# --- CLI Interface ---

def main_cli():
    parser = argparse.ArgumentParser(
        description="Relic Materializer v3.0: A schema-aware engine to create or modify Relics.",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("plan_file", type=Path, help="Path to the Relic Plan JSON file.")
    parser.add_argument("--output-dir", "-o", type=Path, default=Path("."), help="Base directory for the project.")
    parser.add_argument("--force", "-f", action="store_true", help="For Full Plans, annihilate and rebuild the target directory if it exists.")
    args = parser.parse_args()

    if not args.plan_file.is_file(): _log_error(f"Plan file not found: '{args.plan_file}'")
    execute_plan(args.plan_file, args.output_dir, args.force)

if __name__ == "__main__":
    main_cli()
