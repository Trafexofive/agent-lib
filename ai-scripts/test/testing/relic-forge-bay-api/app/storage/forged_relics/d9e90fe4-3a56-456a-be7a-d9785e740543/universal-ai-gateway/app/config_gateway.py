from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import List, Optional

class Settings(BaseSettings):
    APP_NAME: str = "Universal AI Gateway"
    APP_VERSION: str = "0.1.0"
    APP_PORT_INTERNAL: int = 8011 # Port uvicorn listens on *inside* the container
    LOG_LEVEL: str = "INFO"

    CORS_ALLOWED_ORIGINS: str = "http://localhost,http://localhost:8011,http://127.0.0.1:8011"

    # Provider API Keys - User MUST provide these in .env for actual integration
    GEMINI_API_KEY: Optional[str] = None
    GROQ_API_KEY: Optional[str] = None
    # OTHER_FREE_PROVIDER_API_KEY: Optional[str] = None

    # Provider API Endpoints (can be overridden in .env if necessary)
    GEMINI_API_ENDPOINT: str = "https://generativelanguage.googleapis.com/v1beta/models"
    GROQ_API_ENDPOINT: str = "https://api.groq.com/openai/v1"
    # OTHER_FREE_PROVIDER_ENDPOINT: Optional[str] = None

    # Default models per provider (can be overridden in .env)
    GEMINI_DEFAULT_MODEL: str = "gemini-1.5-flash-latest"
    GROQ_DEFAULT_MODEL: str = "llama3-8b-8192" # Groq model identifier
    # OTHER_FREE_PROVIDER_DEFAULT_MODEL: Optional[str] = None

    DEFAULT_PROVIDER_TIMEOUT: int = 30 # Default timeout for HTTP requests to AI providers

    @property
    def CORS_ALLOWED_ORIGINS_LIST(self) -> List[str]:
        if not self.CORS_ALLOWED_ORIGINS:
            return []
        return [origin.strip() for origin in self.CORS_ALLOWED_ORIGINS.split(',') if origin.strip()]

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra='ignore'
    )

settings = Settings()
