import os
from fastapi import FastAPI, HTTPException, Body
from pydantic import BaseModel
from typing import Dict, Any

from agent_runner import AgentRunner, AgentNotFoundException

# -- Configuration --
RELIC_NAME = "AgentFactoryRelic"
RELIC_VERSION = "0.1.0"

app = FastAPI(
    title=RELIC_NAME,
    version=RELIC_VERSION,
    description="An API for executing YAML-defined autonomous agents via an LLM service like Ollama."
)

agent_runner = AgentRunner()

# -- Pydantic Models --
class ExecutionRequest(BaseModel):
    inputs: Dict[str, Any]

class ExecutionResponse(BaseModel):
    output: Dict[str, Any]
    message: str

# -- API Endpoints --

@app.get("/context", tags=["Agent Interaction"], summary="Get Relic's operational context")
def get_context():
    """Provides essential context about the relic for other agents and for manual inspection."""
    return {
        "relic_name": RELIC_NAME,
        "version": RELIC_VERSION,
        "description": "This relic executes agents defined in YAML files. Use the /agents/{agent_name}/execute endpoint to run an agent.",
        "system_prompt_fragment": (
            "To use this Agent Factory, you must POST to the endpoint `/api/v1/agents/{agent_name}/execute`. "
            "The `{agent_name}` corresponds to a YAML file in the `agent_configs` directory (e.g., 'code-generator'). "
            "The request body must be a JSON object with an 'inputs' key, which itself is an object containing the dynamic values for the agent's persona prompt. "
            "Example for 'code-generator': `{\"inputs\": {\"user_request\": \"Create a python script that prints hello world\"}}`."
        ),
        "capabilities": [
            "Dynamically loads agent configurations from YAML files.",
            "Interfaces with an Ollama backend to get agentic responses.",
            "Executes predefined tools based on the LLM's output."
        ],
        "status_notes": ["Operational. Backend only. Ready for frontend integration."]
    }

@app.post("/api/v1/agents/{agent_name}/execute", response_model=ExecutionResponse, tags=["Agent Execution"])
async def execute_agent(agent_name: str, request: ExecutionRequest):
    """Executes a specific agent based on its YAML configuration."""
    try:
        print(f"Executing agent '{agent_name}' with inputs: {request.inputs}")
        result = await agent_runner.run(agent_name, request.inputs)
        return {
            "output": result,
            "message": f"Agent '{agent_name}' executed successfully."
        }
    except AgentNotFoundException as e:
        raise HTTPException(status_code=404, detail=str(e))
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        raise HTTPException(status_code=500, detail=f"An internal error occurred: {str(e)}")


# Health check endpoint
@app.get("/health", tags=["System"])
def health_check():
    return {"status": "ok"}
