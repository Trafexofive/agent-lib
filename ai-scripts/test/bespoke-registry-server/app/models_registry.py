from pydantic import BaseModel, Field, Json
from typing import Optional, List, Any, Dict
import datetime
import uuid

from sqlalchemy import Column, String, Text, Float, DateTime, Integer
from sqlalchemy.orm import declarative_base
from sqlalchemy.types import JSON # For storing list of tags as JSON string

Base = declarative_base()

# --- SQLAlchemy Model ---_-
class AssetMetadataDB(Base):
    __tablename__ = "assets"

    id: str = Column(String, primary_key=True, default=lambda: str(uuid.uuid4()), index=True)
    original_filename: str = Column(String, nullable=False)
    stored_filename: str = Column(String, nullable=False, unique=True) # e.g., UUID based name on disk
    content_type: Optional[str] = Column(String)
    size_bytes: Optional[int] = Column(Integer)
    upload_timestamp: datetime.datetime = Column(DateTime(timezone=True), default=lambda: datetime.datetime.now(datetime.timezone.utc))
    user_description: Optional[str] = Column(Text)
    
    # Gemini Enhanced Fields (simulated)
    auto_tags: Optional[List[str]] = Column(JSON) # Store as JSON list of strings
    quality_rating: Optional[float] = Column(Float) # e.g., 0.0 to 1.0 or 1-5
    suggested_name: Optional[str] = Column(String)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "original_filename": self.original_filename,
            "stored_filename": self.stored_filename,
            "content_type": self.content_type,
            "size_bytes": self.size_bytes,
            "upload_timestamp": self.upload_timestamp.isoformat() if self.upload_timestamp else None,
            "user_description": self.user_description,
            "auto_tags": self.auto_tags,
            "quality_rating": self.quality_rating,
            "suggested_name": self.suggested_name
        }

# --- Pydantic Models for API ---_-

class AssetMetadataBase(BaseModel):
    original_filename: str
    content_type: Optional[str] = None
    size_bytes: Optional[int] = None
    user_description: Optional[str] = None
    auto_tags: Optional[List[str]] = None
    quality_rating: Optional[float] = None
    suggested_name: Optional[str] = None

class AssetMetadataResponse(AssetMetadataBase):
    id: str # UUID as string
    stored_filename: str
    upload_timestamp: datetime.datetime
    download_url: str

    model_config = {
        "from_attributes": True # SQLAlchemy model to Pydantic
    }

# For internal representation of Gemini's (simulated) analysis
class GeminiAnalysisResult(BaseModel):
    auto_tags: List[str] = Field(default_factory=list)
    quality_rating: Optional[float] = None
    suggested_name: Optional[str] = None

# For error responses
class ErrorDetailResponse(BaseModel):
    detail: str

# For system info endpoints
class HealthStatus(BaseModel):
    app_name: str
    version: str
    status: str
    timestamp: datetime.datetime

class CapabilityDetail(BaseModel):
    method: str
    path: str
    description: str

class SystemCapabilities(BaseModel):
    app_name: str
    version: str
    description: str
    capabilities: List[CapabilityDetail]
    gemini_integration_status: str
