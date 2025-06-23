import logging, asyncio
from datetime import datetime
from .loader import ComponentLoader
from .tools import ToolRegistry, ActionContext
from .agent_runner import AgentRunner

logger = logging.getLogger(__name__)

class OrchestrationEngine:
    def __init__(self, loader: ComponentLoader, tool_registry: ToolRegistry):
        self.loader = loader; self.tool_registry = tool_registry
        self.event_queue = asyncio.Queue()

    async def ingest_webhook_event(self, webhook_id, payload):
        await self.event_queue.put({'type': 'webhook', 'id': webhook_id, 'payload': payload})

    async def run(self):
        logger.info("Engine run loop started. Awaiting events.")
        asyncio.create_task(self.time_trigger_producer())
        while True:
            event = await self.event_queue.get()
            asyncio.create_task(self.process_event(event))

    async def time_trigger_producer(self):
        last_minute = -1
        while True:
            now = datetime.now()
            if now.minute != last_minute:
                last_minute = now.minute
                await self.event_queue.put({'type': 'time_pattern', 'timestamp': now.isoformat()})
            await asyncio.sleep(1)

    async def process_event(self, event):
        for orch in self.loader.orchestrations.values():
            for trigger in orch.get('triggers', []):
                if self._trigger_matches(trigger, event):
                    logger.info(f"TRIGGER FIRED: '{orch.get('alias')}' by {event['type']}.")
                    await self.execute_orchestration(orch, event)

    def _trigger_matches(self, trigger, event):
        if trigger.get('platform') != event['type']: return False
        if event['type'] == 'webhook': return trigger.get('id') == event['id']
        if event['type'] == 'time_pattern': return trigger.get('minutes') == '*' # Simplified
        return False

    async def execute_orchestration(self, orch, trigger_event):
        context = ActionContext(trigger_data=trigger_event)
        for action_def in orch.get('actions', []):
            action_type = next(iter(action_def))
            action_params = action_def[action_type]
            if action_type == 'service': await self._execute_service(action_params, context)
            elif action_type == 'agent': await self._execute_agent(action_params, context)

    async def _execute_service(self, params, context):
        tool_id = context.render_template(params['id'])
        tool_params = {k: context.render_template(v) for k, v in params.get('data', {}).items()}
        if not (tool := self.tool_registry.get_tool(tool_id)): return logger.error(f"Tool '{tool_id}' not found.")
        try:
            result = await tool.execute(tool_params)
            if res_var := params.get('response_variable'): context.set_variable(res_var, result)
        except Exception as e: logger.error(f"Tool '{tool_id}' failed: {e}")

    async def _execute_agent(self, params, context):
        agent_id = context.render_template(params['id'])
        goal = context.render_template(params['goal'])
        if not (profile := self.loader.agents.get(agent_id)): return logger.error(f"Agent '{agent_id}' not found.")
        runner = AgentRunner(profile, self.tool_registry, self.loader)
        result = await runner.run(goal, context)
        if res_var := params.get('response_variable'): context.set_variable(res_var, result)
