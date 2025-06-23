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
        self.tools: Dict[str, Any] = {}
        if not self.workspace_path.is_dir(): raise FileNotFoundError(f"Workspace DNE: {workspace_path}")

    def load_all(self):
        logger.info(f"Loading components from: {self.workspace_path}")
        self.agents.clear(); self.workflows.clear(); self.orchestrations.clear(); self.tools.clear()
        self._load_dir(self.workspace_path / "agents", self._process_component, self.agents)
        self._load_dir(self.workspace_path / "workflows", self._process_component, self.workflows)
        self._load_dir(self.workspace_path / "orchestrations", self._process_component, self.orchestrations)
        self._load_dir(self.workspace_path / "tools", self._process_component, self.tools)
        logger.info(f"Load complete. Found {len(self.tools)} tools, {len(self.agents)} agents.")

    def _load_dir(self, path, processor, target_dict):
        if not path.is_dir(): return
        for f in path.glob("**/*.yaml"): 
            try: processor(yaml.safe_load(f.read_text()), target_dict)
            except Exception as e: logger.error(f"Failed to process {f.name}: {e}")

    def _process_component(self, content, target_dict):
        docs = content if isinstance(content, list) else [content]
        for doc in docs: 
            if id := doc.get('id'): target_dict[id] = doc
