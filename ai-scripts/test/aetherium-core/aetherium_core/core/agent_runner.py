import logging, json, re
from typing import Dict, Any, List
from .tools import ToolRegistry, ActionContext
from .loader import ComponentLoader
from aetherium_core.integrations.llm.factory import LLMClientFactory

logger = logging.getLogger(__name__)

class AgentRunner:
    def __init__(self, profile: Dict[str, Any], tool_registry: ToolRegistry, loader: ComponentLoader):
        self.profile = profile; self.tool_registry = tool_registry; self.loader = loader
        self.model_name = self.profile['memory']['short_term']['reasoning_model']
        self.llm_client = LLMClientFactory.get_client(self.model_name)

    async def run(self, goal: str, context: ActionContext) -> Dict[str, Any]:
        agent_id = self.profile['id']
        logger.info(f"Agent '{agent_id}' starting run. Goal: {goal}")
        system_prompt = self._build_system_prompt()
        history = [{'role': 'user', 'content': goal}]
        max_iter = self.profile['execution_plan'].get('max_iterations', 5)

        for _ in range(max_iter):
            response_text = await self.llm_client.generate_response(system_prompt, history, self.model_name, 0.2)
            history.append({'role': 'assistant', 'content': response_text})
            if '<tool_call>' not in response_text:
                logger.info(f"Agent '{agent_id}' finished.")
                return {'status': 'success', 'content': response_text}
            
            tool_name, tool_params = self._parse_tool_call(response_text)
            if tool_name:
                tool_result = await self._execute_tool(tool_name, tool_params)
                observation = f'<tool_response name="{tool_name}">{json.dumps(tool_result)}</tool_response>'
                history.append({'role': 'user', 'content': observation})
            else:
                history.append({'role': 'user', 'content': '<tool_response>Error: Malformed tool call.</tool_response>'})
        return {'status': 'failure', 'reason': 'Max iterations reached'}

    def _build_system_prompt(self) -> str:
        base_prompt = self.profile['constitution']['system_prompt']
        tools = self.profile['capabilities']['allowed_tools']
        tools_defs = [self.loader.tools.get(t) for t in tools if self.loader.tools.get(t)]
        tools_desc = "\n\nAvailable Tools:\n" + "\n".join([f"- {t['id']}: {t['description']}" for t in tools_defs])
        tool_format = "\nTo use a tool, respond with ONLY: <tool_call>{\"name\": \"tool_id\", \"parameters\": {\"param\": \"value\"}}</tool_call>"
        return base_prompt + tools_desc + tool_format

    def _parse_tool_call(self, text: str):
        match = re.search(r'<tool_call>(.*?)</tool_call>', text, re.DOTALL)
        if not match: return None, None
        try: 
            data = json.loads(match.group(1))
            return data.get('name'), data.get('parameters')
        except (json.JSONDecodeError, AttributeError): return None, None

    async def _execute_tool(self, tool_name: str, params: dict):
        if not (tool := self.tool_registry.get_tool(tool_name)): return {'error': f'Tool "{tool_name}" not found.'}
        try: return await tool.execute(params or {})
        except Exception as e: return {'error': str(e)}
