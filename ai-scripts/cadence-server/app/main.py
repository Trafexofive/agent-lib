from fastapi import FastAPI
from contextlib import asynccontextmanager
import asyncio

from .hearth import Hearth
from .api import router as api_router

@asynccontextmanager
async def lifespan(app: FastAPI):
    # On startup, initialize and run the Hearth engine in the background.
    print("[MAIN] Initializing CADAF Hearth...")
    hearth_instance = Hearth(
        workflows_dir="./workflows",
        secrets_file="./secrets/.env"
    )
    app.state.hearth = hearth_instance
    
    # Start background tasks (scheduler, etc.)
    hearth_task = asyncio.create_task(hearth_instance.start_background_services())
    print("[MAIN] CADAF Hearth is now running in the background.")
    
    yield
    
    # On shutdown
    print("[MAIN] Shutting down CADAF Hearth...")
    hearth_task.cancel()
    try:
        await hearth_task
    except asyncio.CancelledError:
        print("[MAIN] Hearth background services stopped.")

app = FastAPI(
    title="CADAF - Cadence Server",
    version="1.0.0",
    lifespan=lifespan,
    description="The control plane for the Chimera Agent-Driven Automation Fabric."
)

app.include_router(api_router, prefix="/api/v1")

@app.get("/", tags=["Status"])
async def root():
    return {"message": "CADAF Cadence Server is online."}