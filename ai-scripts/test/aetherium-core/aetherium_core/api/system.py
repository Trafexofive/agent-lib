from fastapi import APIRouter, Request, HTTPException
from aetherium_core.core.loader import ComponentLoader

router = APIRouter()

@router.get("/context")
def get_context():
    return {"relic_name": "aetherium-core", "version": "2.0.0"}

@router.get("/reload")
def reload_components(request: Request):
    loader: ComponentLoader = request.app.state.loader
    if not loader: raise HTTPException(503, "Loader not available.")
    loader.load_all()
    return {"status": "success", "loaded": {"agents": len(loader.agents), "tools": len(loader.tools)}}
