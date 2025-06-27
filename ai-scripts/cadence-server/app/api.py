from fastapi import APIRouter, Depends, HTTPException, Request
from .hearth import Hearth

router = APIRouter()

def get_hearth(request: Request) -> Hearth:
    return request.app.state.hearth

@router.get("/workflows", tags=["Workflows"])
async def list_workflows(hearth: Hearth = Depends(get_hearth)):
    return hearth.get_all_workflow_definitions()

@router.get("/workflows/{workflow_name}", tags=["Workflows"])
async def get_workflow(workflow_name: str, hearth: Hearth = Depends(get_hearth)):
    workflow = hearth.get_workflow_definition(workflow_name)
    if not workflow:
        raise HTTPException(status_code=404, detail="Workflow not found")
    return workflow

@router.post("/workflows/{workflow_name}/trigger", status_code=202, tags=["Workflows"])
async def trigger_workflow(workflow_name: str, request: Request, hearth: Hearth = Depends(get_hearth)):
    if workflow_name not in hearth.workflows:
        raise HTTPException(status_code=404, detail="Workflow not found")
    try:
        inputs = await request.json()
    except:
        inputs = {}

    trigger_data = {
        "type": "api",
        "source_ip": request.client.host,
        "inputs": inputs
    }
    # Run in background and return immediately
    hearth.schedule_workflow_run(workflow_name, trigger_data, inputs)
    return {"message": "Workflow triggered successfully.", "workflow": workflow_name, "run_id": "(scheduled)"}

@router.get("/runs", tags=["Runs"])
async def get_recent_runs(hearth: Hearth = Depends(get_hearth)):
    # In a production system, this would read from a persistent log database.
    return hearth.get_run_history()

@router.get("/runs/{run_id}", tags=["Runs"])
async def get_run_details(run_id: str, hearth: Hearth = Depends(get_hearth)):
    # This would also read from a persistent log database.
    details = hearth.get_run_details(run_id)
    if not details:
        raise HTTPException(status_code=404, detail="Run ID not found in history.")
    return details