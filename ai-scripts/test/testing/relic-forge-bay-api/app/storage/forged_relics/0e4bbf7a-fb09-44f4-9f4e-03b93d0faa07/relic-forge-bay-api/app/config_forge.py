from pydantic_settings import BaseSettings, SettingsConfigDict
from pathlib import Path

class Settings(BaseSettings):
    FORGE_API_HOST: str = "0.0.0.0"
    # This is the port the Uvicorn server runs on *inside* the container.
    # The host port mapping is handled by docker-compose.
    FORGE_INTERNAL_PORT: int = 8000 
    
    FORGE_DATABASE_URL: str = "sqlite+aiosqlite:///./storage/forge_db/relics.db"
    FORGED_RELICS_DIR: str = "./storage/forged_relics"
    
    LOG_LEVEL: str = "INFO"

    RELIC_FORGE_VERSION: str = "0.1.2" # Updated version
    RELIC_FORGE_SERVICE_NAME: str = "RelicForgeBayAPI"

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding='utf-8',
        extra='ignore'
    )

settings = Settings()
