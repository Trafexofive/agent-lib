from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import json
import os
import sqlite3
from pathlib import Path

app = FastAPI()

# CORS configuration (allow all for development)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

CONFIG_FILE = Path("config.json")
DB_PATH = Path("data/relic_database.db")

# --- Database Setup ---
def init_db():
    DB_PATH.parent.mkdir(parents=True, exist_ok=True)
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    # Example table, can be expanded by the user
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT
        )
    ''')
    # Check if default config setting exists, if not, insert from config.json
    cursor.execute("SELECT value FROM settings WHERE key = ?", ('sample_setting_1',))
    if cursor.fetchone() is None:
        try:
            config_data = json.loads(CONFIG_FILE.read_text())
            default_sample_setting = config_data.get('sample_setting_1', 'default_db_value')
            cursor.execute("INSERT INTO settings (key, value) VALUES (?, ?)", 
                           ('sample_setting_1', default_sample_setting))
            conn.commit()
        except (FileNotFoundError, json.JSONDecodeError):
            # If config.json is missing or invalid, insert a hardcoded default
            cursor.execute("INSERT INTO settings (key, value) VALUES (?, ?)", 
                           ('sample_setting_1', 'fallback_db_value'))
            conn.commit()
    conn.close()

# --- Configuration Loading ---
def load_config():
    if not CONFIG_FILE.exists():
        default_config = {
            "sample_setting_1": "initial_value_from_code",
            "feature_x_enabled": False,
            "service_name": "ChimeraDash Backend",
            "service_version": "0.1.0"
        }
        save_config(default_config)
        return default_config
    try:
        return json.loads(CONFIG_FILE.read_text())
    except json.JSONDecodeError:
        # Handle corrupted config, load defaults
        print(f"Warning: {CONFIG_FILE} is corrupted. Loading default configuration.")
        default_config = {
            "sample_setting_1": "corrupted_fallback_value", 
            "feature_x_enabled": False, 
            "service_name": "ChimeraDash Backend", 
            "service_version": "0.1.0"
        }
        save_config(default_config) # Overwrite corrupted file
        return default_config

def save_config(data):
    CONFIG_FILE.write_text(json.dumps(data, indent=2))

# Initialize DB and load config at startup
init_db()
config_data = load_config()

# --- Pydantic Models ---
class ConfigUpdate(BaseModel):
    sample_setting_1: str | None = None
    feature_x_enabled: bool | None = None

class ContextResponse(BaseModel):
    name: str
    version: str
    description: str
    system_prompt_fragment: str
    capabilities: list[str]
    status_notes: str

# --- API Endpoints ---
@app.get("/context", response_model=ContextResponse)
async def get_context():
    return {
        "name": "chimeradash_services",
        "version": config_data.get("service_version", "0.1.0"),
        "description": "Backend services for the ChimeraDash application, providing API endpoints for frontend interaction and data management.",
        "system_prompt_fragment": "You are an AI assistant interacting with the ChimeraDash backend. Its primary role is to serve the ChimeraDash frontend application by managing its configuration and potentially other data services. It exposes a configuration API at /api/v1/config.",
        "capabilities": [
            "Provides system context via /context",
            "Manages backend configuration via /api/v1/config (GET, POST)",
            "Serves the ChimeraDash frontend application (user-provided)",
            "Stores basic settings in an SQLite database"
        ],
        "status_notes": "Operational. Backend is running and serving the user-provided frontend. Configuration is loaded."
    }

@app.get("/api/v1/config")
async def get_config():
    # Return only agent-modifiable (Tier 1) settings
    current_config = load_config()
    tier1_settings = {
        "sample_setting_1": current_config.get("sample_setting_1"),
        "feature_x_enabled": current_config.get("feature_x_enabled")
    }
    return tier1_settings

@app.post("/api/v1/config")
async def update_config(update_data: ConfigUpdate):
    current_config = load_config()
    updated = False
    if update_data.sample_setting_1 is not None:
        current_config["sample_setting_1"] = update_data.sample_setting_1
        updated = True
    if update_data.feature_x_enabled is not None:
        current_config["feature_x_enabled"] = update_data.feature_x_enabled
        updated = True
    
    if updated:
        save_config(current_config)
        # Optionally update DB if this setting is also stored there
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        if update_data.sample_setting_1 is not None:
             cursor.execute("UPDATE settings SET value = ? WHERE key = ?", (update_data.sample_setting_1, 'sample_setting_1'))
        conn.commit()
        conn.close()
        return {"message": "Configuration updated successfully", "new_config": current_config}
    else:
        return {"message": "No changes applied to configuration", "current_config": current_config}

# Example endpoint using the database
@app.get("/api/v1/settings/{key}")
async def get_setting_from_db(key: str):
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("SELECT value FROM settings WHERE key = ?", (key,))
    row = cursor.fetchone()
    conn.close()
    if row:
        return {"key": key, "value": row[0]}
    raise HTTPException(status_code=404, detail=f"Setting '{key}' not found in database")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=int(os.getenv("BACKEND_PORT", 8000)))
