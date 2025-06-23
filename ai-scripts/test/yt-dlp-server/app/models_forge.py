from pydantic import BaseModel, Field, field_validator,RootModel
from sqlalchemy import Column, String, DateTime, Text
from sqlalchemy.orm import declarative_base
import datetime
import os # For path validation in Pydantic model
from typing import Optional, List, Dict, Any

Base = declarative_base()

class ArtifactPlanItem(BaseModel):
    title: str
    path: str
    contentType: str
    content: str

    @field_validator('path')
    @classmethod
    def validate_artifact_path(cls, v: str) -> str:
        normalized_path = os.path.normpath(v)
        if os.path.isabs(normalized_path) or ".." in normalized_path.split(os.sep):
            raise ValueError("Artifact path must be relative and not contain '..'")
        if not v.strip(): # Ensure path is not empty or just whitespace
             raise ValueError("Artifact path cannot be empty")
        return v

class RelicPlan(BaseModel):
    relic_name: str = Field(..., min_length=1)
    version: str = Field(..., pattern=r"^\d+\.\d+\.\d+([a-zA-Z0-9.-]*)$") # More flexible SemVer
    bootstrap_script_content: Optional[str] = None
    artifacts: List[ArtifactPlanItem]
    setup_instructions_markdown: Optional[str] = None

# SQLAlchemy model for database storage
class RelicMetadataDB(Base):
    __tablename__ = "relics"

    relic_id: str = Column(String, primary_key=True, index=True)
    relic_name: str = Column(String, nullable=False, index=True)
    package_filename: str = Column(String, nullable=False)
    download_url: str = Column(String, nullable=False) # URL path, not full URL
    status: str = Column(String, nullable=False, default="forged")
    timestamp: str = Column(String, nullable=False) # ISO format string from datetime.utcnow().isoformat()
    version: str = Column(String, nullable=False)
    # Optional: Add Text column for setup_instructions_markdown if desired
    # setup_instructions: Optional[str] = Column(Text, nullable=True)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "relic_id": self.relic_id,
            "relic_name": self.relic_name,
            "package_filename": self.package_filename,
            "download_url": self.download_url,
            "status": self.status,
            "timestamp": self.timestamp,
            "version": self.version
        }

# Pydantic model for API responses (can be same as DB or a subset/superset)
class RelicMetadata(BaseModel):
    relic_id: str
    relic_name: str
    package_filename: str
    download_url: str
    status: str
    timestamp: str # Kept as string for ISO format consistency
    version: str

    # If you want to convert timestamp string to datetime object on Pydantic model creation:
    # @field_validator('timestamp', mode='before')
    # @classmethod
    # def parse_timestamp(cls, value: str) -> datetime.datetime:
    #     if isinstance(value, datetime.datetime):
    #         return value
    #     try:
    #         return datetime.datetime.fromisoformat(value.replace('Z', '+00:00'))
    #     except (ValueError, TypeError):
    #         # Fallback or raise error - depends on how strict you want to be
    #         return datetime.datetime.utcnow() # Or raise error
