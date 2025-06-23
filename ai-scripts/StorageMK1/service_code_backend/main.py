import os
import json
import shutil
import logging
from contextlib import asynccontextmanager
from datetime import datetime
from typing import List, Optional, Dict, Any

from fastapi import FastAPI, UploadFile, File, HTTPException, Query, Body
from fastapi.responses import JSONResponse, FileResponse
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
import aiofiles
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# --- Configuration & Logging --- 
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

CONFIG_FILE_PATH = "app_config.json"

def load_app_config():
    try:
        with open(CONFIG_FILE_PATH, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        logger.warning(f"Configuration file {CONFIG_FILE_PATH} not found. Using defaults.")
        return {
            "storage_base_path_container": "/storage_data",
            "max_upload_size_mb": 1024,
            "default_listing_sort": "name_asc",
            "allow_overwrite_on_upload": False,
            "api_title": "StorageMK1 API (Default Config)",
            "api_version": "0.2.1", # Ensure version is updated
            "hidden_files_prefixes": [".", "~"]
        }

app_config = load_app_config()
STORAGE_BASE = app_config.get("storage_base_path_container")

if not os.path.exists(STORAGE_BASE):
    os.makedirs(STORAGE_BASE, exist_ok=True)
    logger.info(f"Created storage base directory: {STORAGE_BASE}")

# --- Pydantic Models --- 
class RelicContext(BaseModel):
    relic_name: str
    version: str
    description: str
    system_prompt_fragment: str
    capabilities: list[str]
    status_notes: list[str]

class StorageItem(BaseModel):
    name: str
    type: str # 'file' or 'directory'
    size: Optional[int] = None # In bytes, None for directories
    modified_at: datetime
    path: str # Full path relative to storage root, e.g. /documents/file.txt

class BrowseResponse(BaseModel):
    path: str
    items: List[StorageItem]

class MkdirRequest(BaseModel):
    path: str = Field(..., description="Path for the new directory, relative to storage root, e.g., /new_folder or /docs/reports")

class DeleteRequest(BaseModel):
    path: str = Field(..., description="Path of the file or directory to delete, relative to storage root, e.g., /file.txt or /old_folder")

class MoveRequest(BaseModel):
    source_path: str = Field(..., description="Current path of the file/directory, e.g. /old_folder/item.txt")
    destination_path: str = Field(..., description="New path for the file/directory, e.g. /new_folder/renamed_item.txt")

# --- Helper Functions --- 
def get_validated_full_path(user_path: str) -> str:
    if not user_path.startswith('/'):
        user_path = '/' + user_path
    # Normalize path to resolve '..' and '.' and prevent escaping STORAGE_BASE
    # os.path.join ensures correct path separators for the OS FastAPI is running on
    full_path = os.path.normpath(os.path.join(STORAGE_BASE, user_path.lstrip('/')))

    # Security check: ensure the path is still within STORAGE_BASE
    if not full_path.startswith(os.path.normpath(STORAGE_BASE)):
        logger.warning(f"Path traversal attempt detected: {user_path} resolved to {full_path}")
        raise HTTPException(status_code=400, detail="Invalid path: Access denied.")
    return full_path

# --- FastAPI Lifespan --- 
@asynccontextmanager
async def lifespan(app: FastAPI):
    global app_config, STORAGE_BASE
    app_config = load_app_config()
    STORAGE_BASE = app_config.get("storage_base_path_container")
    if not os.path.exists(STORAGE_BASE):
        os.makedirs(STORAGE_BASE, exist_ok=True)
        logger.info(f"Lifespan: Ensured storage base directory exists: {STORAGE_BASE}")
    logger.info(f"StorageMK1 API starting. Storage base: {STORAGE_BASE}. Config loaded. Version: {app_config.get('api_version')}")
    yield
    logger.info("StorageMK1 API shutting down.")

app = FastAPI(
    title=app_config.get("api_title"), 
    version=app_config.get("api_version"),
    lifespan=lifespan
)

# CORS Configuration
_frontend_port_host = os.getenv("FRONTEND_PORT_HOST", "5173") # Default if not in .env
_domain_name = os.getenv("DOMAIN_NAME", "localhost") # Default if not in .env

default_specific_origins = [
    f"http://{_domain_name}:{_frontend_port_host}",
    f"http://localhost:{_frontend_port_host}", # Common for local dev regardless of DOMAIN_NAME
]

_user_defined_cors_origins_str = os.getenv("CORS_ALLOWED_ORIGINS")
if _user_defined_cors_origins_str is not None and _user_defined_cors_origins_str.strip() != "":
    if _user_defined_cors_origins_str == "*":
        origins_to_use = ["*"]
    else:
        # Split by comma and strip whitespace from each origin
        origins_to_use = [origin.strip() for origin in _user_defined_cors_origins_str.split(',') if origin.strip()]
else:
    # Use default if CORS_ALLOWED_ORIGINS is empty or not defined
    origins_to_use = default_specific_origins

logger.info(f"CORS allow_origins configured: {origins_to_use}")

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins_to_use,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# --- API Endpoints --- 
@app.get("/context", response_model=RelicContext)
async def get_context_endpoint():
    return {
        "relic_name": "StorageMK1",
        "version": app_config.get("api_version"),
        "description": "A lightweight, modular mini-NAS relic providing API-driven file and directory management. Designed to be used with a user-provided frontend.",
        "system_prompt_fragment": "To manage files with StorageMK1, an agent can use endpoints under /api/v1/storage/. For example, to list files in '/documents', GET /api/v1/storage/browse?path=/documents. A frontend is expected to be provided by the user.",
        "capabilities": [
            "File and directory listing",
            "File upload and download",
            "Directory creation",
            "File and directory deletion",
            "File and directory move/rename",
            "Configurable storage backend via API (/api/v1/config)",
            "Self-description via /context endpoint",
            "Ready for user-provided Web Frontend integration"
        ],
        "status_notes": [
            f"Operational. Storage root (container): {STORAGE_BASE}.",
            f"Backend API ready. User to provide frontend code and Dockerfile in 'frontend/' directory.",
            f"Frontend expected to be accessible at http://{_domain_name}:{_frontend_port_host} (or http://localhost:{_frontend_port_host} as per .env)"
        ]
    }

@app.get("/api/v1/config")
async def get_config_endpoint():
    return app_config

@app.post("/api/v1/config")
async def update_config_endpoint(new_config: Dict[str, Any]):
    global app_config, STORAGE_BASE
    try:
        # Basic update: only known keys or replace if that's the design
        for key, value in new_config.items():
            if key in app_config:
                app_config[key] = value
        
        with open(CONFIG_FILE_PATH, 'w') as f:
            json.dump(app_config, f, indent=2)
        
        # Re-assign STORAGE_BASE if it changed and ensure it exists
        STORAGE_BASE = app_config.get("storage_base_path_container")
        if not os.path.exists(STORAGE_BASE):
            os.makedirs(STORAGE_BASE, exist_ok=True)

        logger.info(f"App config updated. New storage base: {STORAGE_BASE}")
        return {"message": "Configuration updated successfully.", "new_config": app_config}
    except Exception as e:
        logger.error(f"Error updating configuration: {e}")
        raise HTTPException(status_code=500, detail=f"Failed to update configuration: {str(e)}")

@app.get("/api/v1/storage/browse", response_model=BrowseResponse)
async def browse_path(path: str = Query("/", description="Path to browse, relative to storage root. E.g. / or /documents")):
    full_path = get_validated_full_path(path)
    if not os.path.exists(full_path) or not os.path.isdir(full_path):
        raise HTTPException(status_code=404, detail="Path not found or is not a directory.")

    items = []
    hidden_prefixes = tuple(app_config.get("hidden_files_prefixes", [".", "~"]))
    try:
        for entry in os.listdir(full_path):
            if entry.startswith(hidden_prefixes):
                continue # Skip hidden files/folders
            entry_path = os.path.join(full_path, entry)
            stat_info = os.stat(entry_path)
            item_type = "directory" if os.path.isdir(entry_path) else "file"
            # Ensure consistent path separators (/) and leading slash for relpath
            relative_entry_path = "/" + os.path.relpath(entry_path, STORAGE_BASE).replace("\\\\", "/") # Python: \\ means literal backslash. JSON: \\\\ means literal backslash.
            
            items.append(StorageItem(
                name=entry,
                type=item_type,
                size=stat_info.st_size if item_type == "file" else None,
                modified_at=datetime.fromtimestamp(stat_info.st_mtime),
                path=relative_entry_path
            ))
        # Sort items (example: by type then name)
        items.sort(key=lambda x: (x.type, x.name.lower()))
        # Normalize current path for response
        current_relative_path = "/" + os.path.relpath(full_path, STORAGE_BASE).replace("\\\\", "/")
        if current_relative_path == "/.": current_relative_path = "/" # Normalize root path case

        return BrowseResponse(path=current_relative_path, items=items)
    except OSError as e:
        logger.error(f"Error browsing path {full_path}: {e}")
        raise HTTPException(status_code=500, detail=f"Error accessing path: {e.strerror}")

@app.post("/api/v1/storage/upload")
async def upload_file_endpoint(
    path: str = Query("/", description="Directory to upload the file to, relative to storage root."), 
    file: UploadFile = File(...)
):
    upload_dir_full_path = get_validated_full_path(path)
    if not os.path.isdir(upload_dir_full_path):
        raise HTTPException(status_code=400, detail="Upload path is not a valid directory.")

    file_path = os.path.join(upload_dir_full_path, file.filename)
    
    if os.path.exists(file_path) and not app_config.get("allow_overwrite_on_upload"):
        raise HTTPException(status_code=409, detail=f"File '{file.filename}' already exists. Overwrite not allowed.")

    max_size_bytes = app_config.get("max_upload_size_mb", 1024) * 1024 * 1024
    
    try:
        file_size = 0
        async with aiofiles.open(file_path, 'wb') as out_file:
            while content := await file.read(1024 * 1024): # Read in 1MB chunks
                file_size += len(content)
                if file_size > max_size_bytes:
                    # Clean up partially written file
                    await out_file.close() # Close before attempting to remove
                    if os.path.exists(file_path):
                         os.remove(file_path)
                    raise HTTPException(status_code=413, detail=f"File exceeds maximum size of {app_config.get('max_upload_size_mb')}MB.")
                await out_file.write(content)
        relative_file_path = "/" + os.path.relpath(file_path, STORAGE_BASE).replace("\\\\", "/")
        return {"message": "File uploaded successfully", "filename": file.filename, "path": relative_file_path, "size": file_size}
    except HTTPException: # Re-raise HTTP exceptions (like 413)
        raise
    except OSError as e:
        logger.error(f"Error uploading file to {file_path}: {e}")
        raise HTTPException(status_code=500, detail=f"Could not save file: {e.strerror}")
    finally:
        await file.close()

@app.get("/api/v1/storage/download")
async def download_file_endpoint(filepath: str = Query(..., description="Path to the file to download, relative to storage root.")):
    full_path = get_validated_full_path(filepath)
    if not os.path.exists(full_path) or not os.path.isfile(full_path):
        raise HTTPException(status_code=404, detail="File not found.")
    try:
        return FileResponse(full_path, filename=os.path.basename(full_path))
    except OSError as e:
        logger.error(f"Error downloading file {full_path}: {e}")
        raise HTTPException(status_code=500, detail=f"Error accessing file: {e.strerror}")

@app.post("/api/v1/storage/mkdir")
async def create_directory_endpoint(request: MkdirRequest):
    full_path = get_validated_full_path(request.path)
    if os.path.exists(full_path):
        raise HTTPException(status_code=409, detail="Directory or file already exists at this path.")
    try:
        os.makedirs(full_path, exist_ok=True) # exist_ok=True can be debated for mkdir strictness
        relative_created_path = "/" + os.path.relpath(full_path, STORAGE_BASE).replace("\\\\", "/")
        return {"message": "Directory created successfully", "path": relative_created_path}
    except OSError as e:
        logger.error(f"Error creating directory {full_path}: {e}")
        raise HTTPException(status_code=500, detail=f"Could not create directory: {e.strerror}")

@app.delete("/api/v1/storage/delete")
async def delete_item_endpoint(request: DeleteRequest):
    full_path = get_validated_full_path(request.path)
    if not os.path.exists(full_path):
        raise HTTPException(status_code=404, detail="File or directory not found.")
    try:
        if os.path.isdir(full_path):
            shutil.rmtree(full_path)
            message = "Directory deleted successfully"
        else:
            os.remove(full_path)
            message = "File deleted successfully"
        return {"message": message, "path": request.path}
    except OSError as e:
        logger.error(f"Error deleting {full_path}: {e}")
        raise HTTPException(status_code=500, detail=f"Could not delete item: {e.strerror}")

@app.post("/api/v1/storage/move")
async def move_item_endpoint(request: MoveRequest):
    source_full_path = get_validated_full_path(request.source_path)
    destination_full_path = get_validated_full_path(request.destination_path)

    if not os.path.exists(source_full_path):
        raise HTTPException(status_code=404, detail=f"Source path '{request.source_path}' not found.")
    
    # Prevent moving a directory into itself or renaming to the same name effectively
    if os.path.normpath(source_full_path) == os.path.normpath(destination_full_path):
        # If source and destination are the same after normalization, it's a no-op.
        return {"message": "Source and destination are the same. No action taken.", "source_path": request.source_path, "destination_path": request.destination_path}

    if os.path.exists(destination_full_path):
      if not app_config.get("allow_overwrite_on_upload"): # Reuse this config for move/rename overwrite
          raise HTTPException(status_code=409, detail=f"Destination path '{request.destination_path}' already exists. Overwrite not allowed.")
      else: # If overwrite is allowed, remove the destination first
          try:
              if os.path.isdir(destination_full_path):
                  shutil.rmtree(destination_full_path)
              else:
                  os.remove(destination_full_path)
              logger.info(f"Overwrite enabled: Removed existing item at destination '{request.destination_path}'")
          except OSError as e:
              logger.error(f"Error removing item at destination {destination_full_path} for overwrite: {e}")
              raise HTTPException(status_code=500, detail=f"Could not overwrite existing item at destination: {e.strerror}")
    
    # Ensure destination directory exists if moving into a new subfolder (e.g. renaming a/b.txt to a/c/d.txt)
    dest_dir = os.path.dirname(destination_full_path)
    if not os.path.exists(dest_dir):
        try:
            os.makedirs(dest_dir, exist_ok=True)
        except OSError as e:
            logger.error(f"Error creating destination directory {dest_dir} for move: {e}")
            raise HTTPException(status_code=500, detail=f"Could not create destination directory: {e.strerror}")
            
    try:
        shutil.move(source_full_path, destination_full_path)
        relative_dest_path = "/" + os.path.relpath(destination_full_path, STORAGE_BASE).replace("\\\\", "/")
        return {"message": "Item moved/renamed successfully", "source_path": request.source_path, "destination_path": relative_dest_path}
    except OSError as e:
        logger.error(f"Error moving {source_full_path} to {destination_full_path}: {e}")
        raise HTTPException(status_code=500, detail=f"Could not move item: {e.strerror}")
