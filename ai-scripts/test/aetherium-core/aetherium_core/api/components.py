from fastapi import APIRouter, Request, HTTPException
from aetherium_core.core.loader import ComponentLoader

router = APIRouter()

# Helper function to get the loader from the app state
def get_loader(request: Request) -> ComponentLoader:
    loader: ComponentLoader = request.app.state.loader
    if not loader: raise HTTPException(503, "ComponentLoader not available.")
    return loader

@router.get("/agents")
def list_agents(request: Request):
    """List all loaded agent IDs."""
    loader = get_loader(request)
    return {"agents": list(loader.agents.keys())}

@router.get("/agents/{agent_id}")
def get_agent_definition(agent_id: str, request: Request):
    """Get the full YAML definition for a specific agent."""
    loader = get_loader(request)
    if agent_id not in loader.agents:
        raise HTTPException(status_code=404, detail=f"Agent '{agent_id}' not found.")
    return loader.agents[agent_id]

@router.get("/tools")
def list_tools(request: Request):
    """List all loaded tool IDs."""
    loader = get_loader(request)
    return {"tools": list(loader.tools.keys())}

@router.get("/tools/{tool_id}")
def get_tool_definition(tool_id: str, request: Request):
    """Get the full YAML definition for a specific tool."""
    loader = get_loader(request)
    if tool_id not in loader.tools:
        raise HTTPException(status_code=404, detail=f"Tool '{tool_id}' not found.")
    return loader.tools[tool_id]

@router.get("/orchestrations")
def list_orchestrations(request: Request):
    """List all loaded orchestration IDs."""
    loader = get_loader(request)
    return {"orchestrations": list(loader.orchestrations.keys())}
