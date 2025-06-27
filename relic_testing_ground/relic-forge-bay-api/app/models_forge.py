from pydantic import BaseModel, Field, validator, field_validator
from typing import List, Optional, Any
import datetime
import uuid
import re

from sqlalchemy import Column, String, DateTime, Text, UniqueConstraint # Retained for ORM base if needed
from sqlalchemy.orm import declarative_base # Retained for ORM base if needed

# This Base is for SQLAlchemy ORM. If not using full ORM features for requests/responses,
# it might not be strictly necessary here but is kept for consistency with storage_manager.
DbBase = declarative_base()

class RelicMetadataInDB(DbBase):
    __tablename__ = "forged_relics"

    id: str = Column(String, primary_key=True, index=True)
    relic_name: str = Column(String, index=True, nullable=False)
    version: str = Column(String, nullable=False)
    package_filename: str = Column(String, nullable=False)
    package_path: str = Column(String, nullable=False) 
    status: str = Column(String, default="forged")
    timestamp_utc: datetime.datetime = Column(DateTime(timezone=True), default=lambda: datetime.datetime.now(datetime.timezone.utc))

    __table_args__ = (UniqueConstraint('relic_name', 'version', name='uq_relic_name_version'),)

    def __repr__(self):
        return f"<RelicMetadataInDB(id='{self.id}', name='{self.relic_name}', version='{self.version}')>"

class ArtifactPlan(BaseModel):
    title: str = Field(..., description="Human-readable title for the artifact.", examples=["Main Application File"])
    path: str = Field(..., description="Relative path for the artifact. Must not contain '..' or be absolute.", examples=["src/main.py"])
    contentType: str = Field(..., description="MIME type or content type.", examples=["text/python"])
    content: str = Field(..., description="Complete, escaped content of the artifact.")

    @field_validator('path')
    @classmethod
    def validate_path(cls, v: str) -> str:
        if '..' in v or v.startswith('/') or v.startswith('\\'):
            raise ValueError("Artifact path must be relative and not contain '..'")
        if not v.strip():
            raise ValueError("Artifact path cannot be empty or just whitespace")
        return v

class FullRelicPlanInput(BaseModel):
    relic_name: str = Field(..., description="Name of the relic.", min_length=1, max_length=100, examples=["MyAwesomeService"])
    version: str = Field(..., description="Version (e.g., SemVer).", examples=["0.1.0"], pattern=r"^[0-9]+\.[0-9]+\.[0-9]+(?:-[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*)?(?:\+[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*)?$")
    bootstrap_script_content: Optional[str] = Field(None, description="Content of a pre-artifact script (simulated).", examples=["#!/bin/bash\necho 'Bootstrapping...'"])
    artifacts: List[ArtifactPlan] = Field(..., description="Artifacts to create.")
    setup_instructions_markdown: Optional[str] = Field(None, description="Markdown setup instructions.", examples=["# Setup\n1. Unpack.\n2. Run."])

    @field_validator('relic_name')
    @classmethod
    def validate_relic_name(cls, v: str) -> str:
        if not re.match(r"^[a-zA-Z0-9_\-]+$", v):
            raise ValueError("Relic name can only contain alphanumeric characters, underscores, and hyphens.")
        return v

class RelicForgedResponse(BaseModel):
    relic_id: uuid.UUID
    relic_name: str
    version: str
    package_filename: str
    download_url: str
    status: str
    timestamp: datetime.datetime

class RelicMetadata(BaseModel):
    relic_id: uuid.UUID
    relic_name: str
    version: str
    package_filename: str
    download_url: str
    status: str
    timestamp: datetime.datetime

class SystemHealth(BaseModel):
    status: str = Field("OK")
    timestamp: datetime.datetime = Field(default_factory=lambda: datetime.datetime.now(datetime.timezone.utc))
    service_name: str
    version: str

class CapabilityDetail(BaseModel):
    endpoint: str
    method: str
    description: str
    request_body_schema: Optional[Any] = None
    response_schema: Optional[Any] = None

class SystemCapabilities(BaseModel):
    service_name: str
    version: str
    capabilities: List[CapabilityDetail]
    input_formats_accepted: List[str] = ["application/json"]
    output_formats_provided: List[str] = ["application/json", "application/gzip"]

class HTTPErrorDetail(BaseModel):
    message: str
    detail: Optional[Any] = None
