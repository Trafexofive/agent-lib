import asyncio
import json
import os
import shutil
import subprocess
import tempfile
import uuid
from datetime import datetime
from pathlib import Path
from collections import deque
import yaml
from dotenv import dotenv_values

from .context import WorkflowContext
from . import builtins

class Hearth:
    """The core engine for the Chimera Agent-Driven Automation Fabric (CADAF)."""

    def __init__(self, workflows_dir: str, secrets_file: str):
        self.workflows_dir = Path(workflows_dir)
        self.secrets = self._load_secrets(secrets_file)
        self.workflows = self._load_workflows()
        self.run_history = deque(maxlen=100)

        self.built_in_tools = {
            "log": builtins.log,
            "http_request": builtins.http_request,
            "data_transform": builtins.data_transform
        }

    def _load_secrets(self, secrets_file: str) -> dict:
        print(f"[HEARTH] Loading secrets from {secrets_file}")
        try:
            return dict(dotenv_values(secrets_file))
        except Exception as e:
            print(f"[ERROR] Could not load secrets from '{secrets_file}': {e}")
            return {}

    def _load_workflows(self) -> dict:
        print(f"[HEARTH] Loading workflows from {self.workflows_dir}")
        workflows = {}
        for file_path in self.workflows_dir.glob("*.workflow.yml"):
            try:
                with open(file_path, 'r') as f:
                    workflow_def = yaml.safe_load(f)
                    name = workflow_def.get("metadata", {}).get("name")
                    if name:
                        workflows[name] = workflow_def
                        print(f"  - Loaded workflow: '{name}'")
            except Exception as e:
                print(f"[ERROR] Failed to load workflow {file_path.name}: {e}")
        return workflows

    def schedule_workflow_run(self, workflow_name: str, trigger_data: dict, inputs: dict = None):
        asyncio.create_task(self.run_workflow(workflow_name, trigger_data, inputs))

    async def run_workflow(self, workflow_name: str, trigger_data: dict, inputs: dict = None):
        run_id = f"run-{uuid.uuid4().hex[:8]}"
        print(f"[{run_id}] Starting workflow '{workflow_name}'...")

        workflow_def = self.workflows.get(workflow_name)
        if not workflow_def:
            print(f"[{run_id}] ERROR: Workflow '{workflow_name}' not found.")
            return

        context = self._initialize_context(workflow_def, run_id, trigger_data, inputs)
        workspace_dir = tempfile.mkdtemp(prefix=f"cadence-{run_id}-")
        
        try:
            job_graph = {job['id']: job.get('depends_on', []) for job in workflow_def.get('jobs', [])}
            sorted_job_ids = self._topological_sort(job_graph)
            
            for job_id in sorted_job_ids:
                job_def = next((j for j in workflow_def['jobs'] if j['id'] == job_id), None)
                if not job_def: continue

                print(f"[{run_id}][{job_id}] Starting job...")
                job_result = await self.execute_job(job_def, context, workspace_dir)
                context.set(f"jobs.{job_id}", job_result)
                if job_result["status"] == "FAILURE":
                    print(f"[{run_id}][{job_id}] Job failed. Halting workflow.")
                    break
            
            print(f"[{run_id}] Workflow '{workflow_name}' finished.")
            self.log_run_history(run_id, context.data)
        finally:
            shutil.rmtree(workspace_dir)
            print(f"[{run_id}] Workspace cleaned up.")

    def _initialize_context(self, workflow_def, run_id, trigger_data, inputs):
        return WorkflowContext({
            "workflow": {"name": workflow_def.get("metadata", {}).get("name"), "run_id": run_id},
            "trigger": trigger_data,
            "inputs": inputs or trigger_data.get("inputs", {}),
            "secrets": self.secrets,
            "jobs": {}
        })

    async def execute_job(self, job_def: dict, context: WorkflowContext, workspace_dir: str):
        job_id = job_def["id"]
        job_context = WorkflowContext(context.data.copy())
        job_context.set("vars", context.resolve_dict(job_def.get("vars", {})))
        job_context.set("job", {"id": job_id})

        step_outputs = {}
        final_status = "SUCCESS"
        for step_def in job_def.get("steps", []):
            step_id = step_def["id"]
            
            # Basic step dependency check
            if 'depends_on' in step_def:
                for dep_id in step_def['depends_on']:
                    if step_outputs.get(dep_id, {}).get('status') != "SUCCESS":
                        final_status = "FAILURE"
                        print(f"[{context.get('workflow.run_id')}][{job_id}][{step_id}] Skipping due to failed dependency '{dep_id}'.")
                        break
            if final_status == "FAILURE": break

            print(f"[{context.get('workflow.run_id')}][{job_id}][{step_id}] Executing step...")
            step_result = await self.execute_step(step_def, job_context, workspace_dir)
            step_outputs[step_id] = step_result
            job_context.set(f"steps.{step_id}", step_result)
            if step_result["status"] == "FAILURE":
                final_status = "FAILURE"
                print(f"[{context.get('workflow.run_id')}][{job_id}][{step_id}] Step failed. Halting job.")
                break
        
        return {"status": final_status, "steps": step_outputs}

    async def execute_step(self, step_def: dict, context: WorkflowContext, workspace_dir: str):
        step_id = step_def.get('id', 'unnamed-step')
        this_context = { "this": { "runtime": { "workflow_run_id": context.get("workflow.run_id"), "job_id": context.get("job.id"), "step_id": step_id } } }
        resolved_params = context.resolve_dict(step_def, extra_context=this_context)

        try:
            # CORRECTED DISPATCH LOGIC
            action = resolved_params.get("uses")
            if action:
                if action.startswith("builtin:"):
                    tool_name = action.split(":", 1)[1]
                    if tool_name in self.built_in_tools:
                        return await self.built_in_tools[tool_name](resolved_params, context)
                    return {"status": "FAILURE", "error": f"Unknown built-in tool: '{tool_name}'"}
                # ... handle external tools here ...
            elif "script" in resolved_params:
                return await self._run_script(resolved_params, context, workspace_dir)
            elif "llm_call" in resolved_params:
                return {"status": "SUCCESS", "result": {"content": "Placeholder LLM response."}, "logs": []}
            elif "tool" in resolved_params:
                 # Logic for external file-based tools
                return await self._run_external_tool(resolved_params, context, workspace_dir)
            else:
                return {"status": "FAILURE", "error": f"No valid action key ('uses', 'script', 'tool', etc.) in step '{step_id}'."}
        except Exception as e:
            return {"status": "FAILURE", "error": f"Unexpected error in step '{step_id}': {e}", "logs": [str(e)]}

    async def _run_script(self, step_def, context, workspace_dir):
        script_content = step_def.get("script")
        resolved_env = {k: str(v) for k, v in step_def.get("env", {}).items()}
        stdin_data = step_def.get("stdin")

        proc = await asyncio.create_subprocess_shell(
            script_content,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            stdin=asyncio.subprocess.PIPE if stdin_data is not None else None,
            env={**os.environ, **resolved_env},
            cwd=workspace_dir
        )
        stdout, stderr = await proc.communicate(input=stdin_data.encode() if stdin_data is not None else None)
        return self._process_script_result(stdout, stderr, proc.returncode)
    
    async def _run_external_tool(self, step_def, context, workspace_dir):
        # This function handles tools like 'filesystem_unrestricted'
        tool_name = step_def["tool"]
        
        # In a real system, we'd look up the tool's script path from a registry.
        # Here we hardcode it for simplicity, assuming a known location.
        tool_script_path = f"/app/tools/{tool_name}.py" # Assuming tools are placed in /app/tools
        if not Path(tool_script_path).exists():
             # Fallback for cadence_cli which is in a different path
             tool_script_path = f"/app/tools/{tool_name}_tool.py"
             if not Path(tool_script_path).exists():
                return {"status":"FAILURE", "error": f"Tool script not found for '{tool_name}'"}

        # Pass all parameters except the 'tool' key itself as a JSON string
        params_for_tool = {k: v for k, v in step_def.items() if k != 'tool'}
        params_json = json.dumps(params_for_tool)

        proc = await asyncio.create_subprocess_exec(
            "python3", tool_script_path, params_json,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            cwd=workspace_dir
        )
        stdout, stderr = await proc.communicate()
        return self._process_script_result(stdout, stderr, proc.returncode)

    def _process_script_result(self, stdout, stderr, returncode):
        logs = []
        if stdout: logs.extend(stdout.decode(errors='ignore').strip().split('\n'))
        if stderr: logs.extend(stderr.decode(errors='ignore').strip().split('\n'))
        
        status = "SUCCESS" if returncode == 0 else "FAILURE"
        
        output_str = stdout.decode(errors='ignore').strip()
        try:
            result = json.loads(output_str)
            # If the result of a tool is JSON and contains a 'success' key,
            # respect its reported status.
            if isinstance(result, dict) and 'success' in result and not result['success']:
                status = "FAILURE"
        except (json.JSONDecodeError, TypeError):
            result = output_str

        error = stderr.decode(errors='ignore').strip() if status == "FAILURE" else None
        return {"status": status, "result": result, "error": error, "logs": logs}

    def _topological_sort(self, graph: dict) -> list:
        # Standard Kahn's algorithm for topological sort
        in_degree = {u: 0 for u in graph}
        for u in graph:
            for v in graph[u]:
                if v in in_degree:
                    in_degree[v] += 1
        
        queue = deque([u for u in in_degree if in_degree[u] == 0])
        sorted_order = []
        
        while queue:
            u = queue.popleft()
            sorted_order.append(u)
            
            for v_id, deps in graph.items():
                if u in deps:
                    in_degree[v_id] -= 1
                    if in_degree[v_id] == 0:
                        queue.append(v_id)
        
        if len(sorted_order) != len(graph):
            # This indicates a cycle in the dependency graph
            cycle_nodes = {k for k,v in in_degree.items() if v > 0}
            print(f"[ERROR] Cycle detected in job dependencies: {cycle_nodes}")
            return [] # Return empty list on failure
        
        return sorted_order

    # API-facing methods
    def get_all_workflow_definitions(self): return self.workflows
    def get_workflow_definition(self, name): return self.workflows.get(name)
    def log_run_history(self, run_id, final_context): self.run_history.appendleft({"run_id": run_id, "context": final_context})
    def get_run_history(self): return list(self.run_history)
    def get_run_details(self, run_id): return next((run for run in self.run_history if run['run_id'] == run_id), None)
