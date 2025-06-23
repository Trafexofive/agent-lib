from pydantic_settings import BaseSettings, SettingsConfigDict

class Settings(BaseSettings):
    APP_PORT: int = 8077
    CHAT_TITLE: str = "The Boys: Diabolical Roundtable (PRIVATE) v0.1.2"
    MAX_HISTORY: int = 100
    LOG_LEVEL: str = "INFO"

    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding='utf-8',
        extra='ignore'
    )

settings = Settings()
