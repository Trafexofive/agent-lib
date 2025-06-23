from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import List

class Settings(BaseSettings):
    app_name: str = "YT-DLP Relic Forger"
    app_version: str = "0.1.1"
    app_env: str = "development"
    app_port_internal: int = 8000 # Port inside the container uvicorn binds to
    
    storage_path: str = "/app/storage"
    max_file_size: int = 1073741824 # Example: 1GB, not actively used by current Relic Forger API
    allowed_origins: str = "http://localhost:8000,http://127.0.0.1:8000"

    # Yt-dlp specific, for future use
    # ytdlp_output_template: str = "%(title)s [%(id)s].%(ext)s"
    # ytdlp_download_archive_file: str = "/app/storage/download_archive.txt"

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra='ignore' # Ignore extra fields from .env
    )

settings = Settings()
