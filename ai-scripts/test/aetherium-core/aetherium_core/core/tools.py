import logging, subprocess, importlib
from abc import ABC, abstractmethod
from typing import Dict, Any
from jinja2 import Template

logger = logging.getLogger(__name__)

class ActionContext:
    def __init__(self, trigger_data: Dict):
        self._variables = {'trigger': trigger_data}
    def set_variable(self, name: str, value: Any): self._variables[name] = value
    def render_template(self, s: Any) -> Any:
        if not isinstance(s, str) or '{{' not in s: return s
        return Template(s, autoescape=False).render(self._variables)

class BaseTool(ABC):
    def __init__(self, definition: Dict): self._definition = definition
    @property
    def name(self) -> str: return self._definition['id']
    @abstractmethod
    async def execute(self, params: Dict[str, Any]) -> Any: pass

class ToolRegistry:
    def __init__(self, tool_definitions: Dict[str, Any]):
        self._tools: Dict[str, BaseTool] = {}
        for definition in tool_definitions.values(): self.register(definition)

    def register(self, definition: Dict):
        runtime = definition.get('runtime', 'python')
        tool_class_map = {'python': PythonTool, 'shell': ShellTool}
        tool_class = tool_class_map.get(runtime)
        if not tool_class: raise ValueError(f"Unsupported runtime: {runtime}")
        tool_instance = tool_class(definition)
        self._tools[tool_instance.name] = tool_instance
        logger.info(f"Tool '{tool_instance.name}' ({runtime}) registered.")

    def get_tool(self, name: str) -> BaseTool | None: return self._tools.get(name)

class PythonTool(BaseTool):
    def __init__(self, definition: Dict):
        super().__init__(definition)
        module_path, class_name = definition['handler'].rsplit('.', 1)
        self.handler_class = getattr(importlib.import_module(module_path), class_name)
        self.instance = self.handler_class(definition)

    async def execute(self, params: Dict[str, Any]) -> Any:
        return await self.instance.execute(params)

class ShellTool(BaseTool):
    async def execute(self, params: Dict[str, Any]) -> Any:
        cmd_template = self._definition['handler']
        # Use Jinja2 for safer, more robust templating than simple replace
        template = Template(cmd_template)
        rendered_cmd = template.render(params=params)
        logger.info(f"Executing shell: {rendered_cmd}")
        result = subprocess.run(rendered_cmd, shell=True, capture_output=True, text=True)
        return {'stdout': result.stdout, 'stderr': result.stderr, 'return_code': result.returncode}

# This class must be imported by main.py to be registered
class LogWriteTool(BaseTool):
    async def execute(self, params: Dict[str, Any]) -> Any:
        logger.info(f"[Tool Execution] {params.get('message')}")
        return {'status': 'logged'}
