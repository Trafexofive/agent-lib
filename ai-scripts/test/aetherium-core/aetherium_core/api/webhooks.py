from fastapi import APIRouter, Request, HTTPException
from aetherium_core.core.engine import OrchestrationEngine

router = APIRouter()

@router.post("/{webhook_id}")
async def handle_webhook(webhook_id: str, request: Request, payload: dict):
    engine: OrchestrationEngine = request.app.state.engine
    if not engine: raise HTTPException(503, "Engine not available.")
    await engine.ingest_webhook_event(webhook_id, payload)
    return {"status": "event received", "webhook_id": webhook_id}
