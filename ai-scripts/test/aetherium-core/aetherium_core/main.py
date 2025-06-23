from fastapi import FastAPI
from aetherium_core.config.logging_config import setup_logging
from aetherium_core.config.cors_config import setup_cors
from aetherium_core.config.app_lifespan import lifespan
from aetherium_core.api import components, system, webhooks

setup_logging()
app = FastAPI(title="Aetherium Core API", version="2.0.0", lifespan=lifespan)
setup_cors(app)

app.include_router(system.router, prefix="/api/v1/system", tags=["System"])
app.include_router(components.router, prefix="/api/v1/components", tags=["Components"])
app.include_router(webhooks.router, prefix="/webhooks", tags=["Webhooks"])

@app.get("/")
def read_root():
    return {"message": "Aetherium Core is online. All systems nominal."}
