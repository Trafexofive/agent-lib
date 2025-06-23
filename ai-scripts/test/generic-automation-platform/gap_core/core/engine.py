import logging, asyncio
from datetime import datetime
from .loader import ComponentLoader
from .tools import ToolRegistry

logger = logging.getLogger(__name__)

class OrchestrationEngine:
    def __init__(self, loader: ComponentLoader, tool_registry: ToolRegistry):
        self.loader = loader
        self.tool_registry = tool_registry
        self.last_checked_minute = -1

    async def run(self):
        logger.info("OrchestrationEngine run loop started.")
        while True:
            await asyncio.sleep(5)
            current_minute = datetime.now().minute
            if current_minute != self.last_checked_minute:
                self.last_checked_minute = current_minute
                await self.process_time_triggers()
    
    async def process_time_triggers(self):
        for orch_key, orchestrations in self.loader.orchestrations.items():
            for orch in orchestrations:
                for trigger in orch.get('trigger', []):
                    if trigger.get('platform') == 'time_pattern' and (trigger.get('minutes') == '*'):
                        logger.info(f"TRIGGER FIRED: {orch.get('alias', orch_key)}")
                        await self.execute_actions(orch)

    async def execute_actions(self, orchestration: dict):
        for action in orchestration.get('action', []):
            service_name = action.get('service')
            if not (tool := self.tool_registry.get_tool(service_name)):
                logger.error(f"Tool '{service_name}' not found.")
                continue
            try:
                logger.info(f"Executing tool: {service_name}")
                await tool.execute(action.get('data', {}))
            except Exception as e:
                logger.error(f"Tool '{service_name}' failed: {e}", exc_info=True)
