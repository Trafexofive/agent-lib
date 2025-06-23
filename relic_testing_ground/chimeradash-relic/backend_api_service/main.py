from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
import json
import os
import sqlite3
from pathlib import Path
from typing import Dict, Any, List, Optional

app = FastAPI(
    title="ChimeraDash Relic Backend API",
    version="0.1.0",
    description="Backend services for the ChimeraDash application."
)

# CORS configuration
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], # Allows all origins
    allow_credentials=True,
    allow_methods=["*"], # Allows all methods
    allow_headers=["*"], # Allows all headers
)

CONFIG_FILE = Path("app_config.json")
DB_PATH = Path("data/chimeradash_relic_data.db")

# --- Database Setup ---
def get_db_connection():
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    conn = get_db_connection()
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS app_settings (
            key TEXT PRIMARY KEY,
            value TEXT,
            last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    # Example: Ensure dashboardTitle from config is in DB
    cursor.execute("SELECT value FROM app_settings WHERE key = ?", ('dashboardTitle',))
    if cursor.fetchone() is None:
        try:
            config_data = json.loads(CONFIG_FILE.read_text())
            default_title = config_data.get('dashboardTitle', 'ChimeraDash Default via DB')
            cursor.execute("INSERT INTO app_settings (key, value) VALUES (?, ?)", 
                           ('dashboardTitle', default_title))
            conn.commit()
        except (FileNotFoundError, json.JSONDecodeError):
            cursor.execute("INSERT INTO app_settings (key, value) VALUES (?, ?)", 
                           ('dashboardTitle', 'Fallback ChimeraDash Title'))
            conn.commit()
    conn.close()

# --- Configuration Loading ---
def load_config() -> Dict[str, Any]:
    if not CONFIG_FILE.exists():
        default_config = {
            "dashboardTitle": "ChimeraDash Initial",
            "defaultTheme": "Chimera Dark Default",
            "featureFlags": {
                "knowledgeGraphEnabled": True,
                "automationServiceEnabled": True,
                "aiToolsEnabled": True
            },
            "service_version": "0.1.0"
        }
        save_config(default_config)
        return default_config
    try:
        return json.loads(CONFIG_FILE.read_text())
    except json.JSONDecodeError:
        print(f"Warning: {CONFIG_FILE} is corrupted. Loading default configuration.")
        default_config = {"dashboardTitle": "Corrupted Config Fallback", "defaultTheme": "Chimera Dark Default", "service_version": "0.1.0"}
        save_config(default_config)
        return default_config

def save_config(data: Dict[str, Any]):
    CONFIG_FILE.write_text(json.dumps(data, indent=2))

# Initialize DB and load config at startup
init_db()
config_data_on_startup = load_config()

# --- Pydantic Models ---
class FeatureFlags(BaseModel):
    knowledgeGraphEnabled: Optional[bool] = None
    automationServiceEnabled: Optional[bool] = None
    aiToolsEnabled: Optional[bool] = None

class ConfigUpdate(BaseModel):
    dashboardTitle: Optional[str] = Field(None, min_length=1, max_length=100)
    defaultTheme: Optional[str] = Field(None, min_length=1, max_length=50)
    featureFlags: Optional[FeatureFlags] = None

class AppConfigResponse(BaseModel):
    dashboardTitle: str
    defaultTheme: str
    featureFlags: FeatureFlags
    service_version: str

class ContextResponse(BaseModel):
    relic_name: str
    version: str
    description: str
    system_prompt_fragment: str
    capabilities: List[str]
    status_notes: str

# --- API Endpoints ---
@app.get("/context", response_model=ContextResponse)
async def get_context_endpoint():
    return {
        "relic_name": "chimeradash-relic",
        "version": load_config().get("service_version", "0.1.0"),
        "description": "Backend API for ChimeraDash, managing configurations and providing data services to the frontend.",
        "system_prompt_fragment": "To use the ChimeraDash relic's API, an agent might query /api/v1/config for application settings. This API serves the ChimeraDash frontend.",
        "capabilities": [
            "Provides ChimeraDash system context via /context",
            "Manages ChimeraDash application configuration via /api/v1/config (GET, POST)",
            "Persists key settings in an SQLite database"
        ],
        "status_notes": "Operational. SQLite for data persistence is active."
    }

@app.get("/api/v1/config", response_model=AppConfigResponse)
async def get_app_config():
    current_config = load_config()
    # Ensure all keys are present, falling back to defaults if necessary
    return AppConfigResponse(
        dashboardTitle=current_config.get("dashboardTitle", "Default Title"),
        defaultTheme=current_config.get("defaultTheme", "Chimera Dark Default"),
        featureFlags=FeatureFlags(**current_config.get("featureFlags", {})),
        service_version=current_config.get("service_version", "0.1.0")
    )

@app.post("/api/v1/config", response_model=AppConfigResponse)
async def update_app_config(update_data: ConfigUpdate):
    current_config = load_config()
    updated_any = False

    if update_data.dashboardTitle is not None:
        current_config["dashboardTitle"] = update_data.dashboardTitle
        conn = get_db_connection()
        cursor = conn.cursor()
        cursor.execute("UPDATE app_settings SET value = ?, last_updated = CURRENT_TIMESTAMP WHERE key = ?", 
                       (update_data.dashboardTitle, 'dashboardTitle'))
        if cursor.rowcount == 0:
            cursor.execute("INSERT INTO app_settings (key, value) VALUES (?, ?)", 
                           ('dashboardTitle', update_data.dashboardTitle))
        conn.commit()
        conn.close()
        updated_any = True
    
    if update_data.defaultTheme is not None:
        current_config["defaultTheme"] = update_data.defaultTheme
        updated_any = True

    if update_data.featureFlags is not None:
        if "featureFlags" not in current_config:
            current_config["featureFlags"] = {}
        for key, value in update_data.featureFlags.model_dump(exclude_unset=True).items():
            current_config["featureFlags"][key] = value
            updated_any = True
    
    if updated_any:
        save_config(current_config)
    
    return AppConfigResponse(
        dashboardTitle=current_config.get("dashboardTitle", "Default Title"),
        defaultTheme=current_config.get("defaultTheme", "Chimera Dark Default"),
        featureFlags=FeatureFlags(**current_config.get("featureFlags", {})),
        service_version=current_config.get("service_version", "0.1.0")
    )

# Example: Get a specific setting from DB (could be expanded)
@app.get("/api/v1/db_settings/{key}")
async def get_db_setting(key: str):
    conn = get_db_connection()
    cursor = conn.cursor()
    cursor.execute("SELECT value, last_updated FROM app_settings WHERE key = ?", (key,))
    row = cursor.fetchone()
    conn.close()
    if row:
        return {"key": key, "value": row["value"], "last_updated": row["last_updated"]}
    raise HTTPException(status_code=404, detail=f"Setting '{key}' not found in database.")

if __name__ == "__main__":
    import uvicorn
    # Ensure data directory exists for SQLite at startup (if Dockerfile didn't handle it)
    DB_PATH.parent.mkdir(parents=True, exist_ok=True) 
    uvicorn.run("main:app", host="0.0.0.0", port=int(os.getenv("BACKEND_PORT_CONTAINER", 8000)), reload=True)
