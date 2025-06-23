import logging, yaml
from pathlib import Path
from typing import Dict, Any

logger = logging.getLogger(__name__)

class ComponentLoader:
    def __init__(self, workspace_path: Path):
        self.workspace_path = workspace_path
        self.agents: Dict[str, Any] = {}
        self.workflows: Dict[str, Any] = {}
        self.orchestrations: Dict[str, Any] = {}
        if not self.workspace_path.is_dir(): raise FileNotFoundError(f"Workspace DNE: {workspace_path}")

    def load_all(self):
        logger.info(f"Loading components from: {self.workspace_path}")
        self.agents.clear(); self.workflows.clear(); self.orchestrations.clear()
        self._load_dir(self.workspace_path / "agents/profiles", self._process_agent)
        self._load_dir(self.workspace_path / "workflows", self._process_workflow)
        self._load_dir(self.workspace_path / "orchestrations", self._process_orchestration)
        logger.info(f"Load complete. Found {len(self.agents)} agents.")

    def _load_dir(self, path, processor):
        if not path.is_dir(): return
        for f in path.glob("*.yaml"): 
            try: processor(yaml.safe_load(f.read_text()), f)
            except Exception as e: logger.error(f"Failed to process {f.name}: {e}")

    def _process_agent(self, content, file_path):
        if agent_id := content.get('metadata', {}).get('id'): self.agents[agent_id] = content

    def _process_workflow(self, content, file_path):
        for wf in content.get('workflows', []): 
            if wf_id := wf.get('id'): self.workflows[wf_id] = wf

    def _process_orchestration(self, content, file_path):
        if orchs := content.get('orchestrations', []): self.orchestrations[file_path.stem] = orchs
