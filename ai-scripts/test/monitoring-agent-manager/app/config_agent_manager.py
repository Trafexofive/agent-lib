from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import List

class Settings(BaseSettings):
    APP_NAME: str = "Monitoring Agent Manager"
    APP_VERSION: str = "0.1.0"
    APP_PORT: int = 8020 # Port for the host if run locally without Docker, Docker overrides via compose
    APP_INTERNAL_PORT: int = 8000 # Port Uvicorn runs on *inside* the container
    LOG_LEVEL: str = "INFO"
    CORS_ALLOWED_ORIGINS: str = "*"

    @property
    def CORS_ALLOWED_ORIGINS_LIST(self) -> List[str]:
        if not self.CORS_ALLOWED_ORIGINS:
            return []
        return [origin.strip() for origin in self.CORS_ALLOWED_ORIGINS.split(',') if origin.strip()]

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra='ignore' # Ignore extra fields from .env
    )

settings = Settings()
