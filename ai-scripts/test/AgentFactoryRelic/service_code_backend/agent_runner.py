import yaml
import os
import json
from typing import Dict, Any

from ollama_client import OllamaClient
from tool_executor import execute_tool

class AgentNotFoundException(Exception):
    pass

class AgentRunner:
    def __init__(self, config_dir="agent_configs"):
        self.config_dir = config_dir
        self.ollama_client = OllamaClient()

    def _load_agent_config(self, agent_name: str) -> Dict[str, Any]:
        config_path = os.path.join(self.config_dir, f"{agent_name}.yaml")
        if not os.path.exists(config_path):
            raise AgentNotFoundException(f"Agent configuration '{config_path}' not found.")
        with open(config_path, 'r') as f:
            return yaml.safe_load(f)

    def _format_prompt(self, template: str, inputs: Dict[str, Any]) -> str:
        for key, value in inputs.items():
            template = template.replace(f'{{{{.{key}}}}}', str(value))
        return template

    async def run(self, agent_name: str, inputs: Dict[str, Any]) -> Dict[str, Any]:
        # 1. Load agent config
        config = self._load_agent_config(agent_name)
        inputs['agent_name'] = config.get('agent_name', agent_name) # Add agent name to inputs
        
        # 2. Format the master prompt
        prompt_template = config['persona']['prompt']
        full_prompt = self._format_prompt(prompt_template, inputs)

        # 3. Get generation from Ollama
        model = config['model']['source']
        params = config['model'].get('parameters', {})
        
        print(f"--- Sending prompt to Ollama model {model} ---")
        print(full_prompt)
        print("--------------------------------------------------")

        llm_response_str = await self.ollama_client.generate(
            model=model,
            prompt=full_prompt,
            temperature=params.get('temperature'),
            stop=params.get('stop_sequences')
        )

        print(f"--- Received raw response from Ollama ---")
        print(llm_response_str)
        print("-------------------------------------------")

        # 4. Parse the LLM's JSON output
        try:
            llm_output = json.loads(llm_response_str)
            tool_call_data = llm_output.get('tool_call')
        except json.JSONDecodeError:
            print("Error: LLM output was not valid JSON.")
            return {"error": "LLM output was not valid JSON", "raw_response": llm_response_str}

        # 5. Execute the tool if specified
        if not tool_call_data:
            print("LLM did not request a tool call.")
            return {"thought": llm_output.get('thought'), "action": "No tool called."}
        
        tool_name = tool_call_data.get('name')
        tool_args = tool_call_data.get('arguments', {})

        if not tool_name:
            return {"error": "LLM output contained a tool_call object with no name."}

        print(f"Executing tool '{tool_name}' with args: {tool_args}")
        tool_result = execute_tool(tool_name, tool_args)
        
        # For now, we return the result of the first tool call. A more complex runner would loop.
        return {
            "thought": llm_output.get('thought'),
            "tool_executed": tool_name,
            "tool_arguments": tool_args,
            "tool_result": tool_result
        }
