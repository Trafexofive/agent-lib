from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import List, Optional

class Settings(BaseSettings):
    APP_NAME: str = "Autonomous Reporter Platform"
    APP_VERSION: str = "0.1.2"
    APP_PORT_INTERNAL: int = 8010
    LOG_LEVEL: str = "INFO"

    PLATFORM_DB_URL_SQLITE_PATH: str = "/app/platform_storage/reports_db/platform.db"
    AGENT_DATA_DIR_CONTAINER: str = "/app/platform_storage/agent_data"

    CORS_ALLOWED_ORIGINS: str = "http://localhost,http://localhost:8010,http://127.0.0.1:8010"

    @property
    def CORS_ALLOWED_ORIGINS_LIST(self) -> List[str]:
        if not self.CORS_ALLOWED_ORIGINS:
            return []
        return [origin.strip() for origin in self.CORS_ALLOWED_ORIGINS.split(',') if origin.strip()]
    
    @property
    def PLATFORM_SQLALCHEMY_DATABASE_URL(self) -> str:
        return f"sqlite+aiosqlite:///{self.PLATFORM_DB_URL_SQLITE_PATH}"

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra='ignore'
    )

settings = Settings()
