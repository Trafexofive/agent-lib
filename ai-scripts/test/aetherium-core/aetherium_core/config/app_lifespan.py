import logging
from contextlib import asynccontextmanager
import asyncio
from pathlib import Path
from fastapi import FastAPI
from aetherium_core.core.loader import ComponentLoader
from aetherium_core.core.engine import OrchestrationEngine
from aetherium_core.core.tools import ToolRegistry

logger = logging.getLogger(__name__)

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("Aetherium Core firing up... FAAFO engaged.")
    try:
        WORKSPACE_DIR = Path("/app/workspace")
        loader = ComponentLoader(WORKSPACE_DIR)
        loader.load_all()
        app.state.loader = loader
        logger.info("ComponentLoader online.")

        tool_registry = ToolRegistry(loader.tools)
        app.state.tool_registry = tool_registry
        logger.info("ToolRegistry online.")

        engine = OrchestrationEngine(loader, tool_registry)
        app.state.engine = engine
        engine_task = asyncio.create_task(engine.run())
        logger.info("OrchestrationEngine online. Systems are live.")

        yield

    finally:
        if 'engine_task' in locals() and not engine_task.done():
            engine_task.cancel()
        logger.info("Aetherium Core powering down.")
