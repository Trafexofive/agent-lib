from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import List, Optional

class Settings(BaseSettings):
    APP_NAME: str = "Bespoke Registry Server"
    APP_VERSION: str = "0.1.0"
    APP_PORT_INTERNAL: int = 8009 # Port uvicorn listens on *inside* the container
    LOG_LEVEL: str = "INFO"

    REGISTRY_DB_URL_SQLITE_PATH: str = "/app/persistent_storage/registry_db/registry.db"
    UPLOADED_ASSETS_DIR_CONTAINER: str = "/app/persistent_storage/uploaded_assets"

    GEMINI_API_KEY: Optional[str] = "YOUR_GEMINI_API_KEY_HERE" # User must set this

    CORS_ALLOWED_ORIGINS: str = "http://localhost,http://localhost:8009,http://127.0.0.1:8009"

    @property
    def CORS_ALLOWED_ORIGINS_LIST(self) -> List[str]:
        if not self.CORS_ALLOWED_ORIGINS:
            return []
        return [origin.strip() for origin in self.CORS_ALLOWED_ORIGINS.split(',') if origin.strip()]
    
    @property
    def REGISTRY_SQLALCHEMY_DATABASE_URL(self) -> str:
        return f"sqlite+aiosqlite:///{self.REGISTRY_DB_URL_SQLITE_PATH}"

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra='ignore'
    )

settings = Settings()
