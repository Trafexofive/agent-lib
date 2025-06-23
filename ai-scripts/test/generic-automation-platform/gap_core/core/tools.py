import logging
from abc import ABC, abstractmethod
from typing import Dict, Any

logger = logging.getLogger(__name__)

class BaseTool(ABC):
    @property
    @abstractmethod
    def name(self) -> str: pass

    @abstractmethod
    async def execute(self, params: Dict[str, Any]): pass

class ToolRegistry:
    def __init__(self):
        self._tools: Dict[str, BaseTool] = {}

    def register(self, tool: BaseTool):
        self._tools[tool.name] = tool
        logger.info(f"Tool '{tool.name}' registered.")

    def get_tool(self, name: str) -> BaseTool | None:
        return self._tools.get(name)

class LogWriteTool(BaseTool):
    @property
    def name(self) -> str: return "log.write"

    async def execute(self, params: Dict[str, Any]):
        message = params.get('message', 'No message provided.')
        level = getattr(logging, params.get('level', 'info').upper(), logging.INFO)
        logger.log(level, f"[Tool Execution] {message}")
