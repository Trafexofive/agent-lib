import datetime
import uuid
from enum import Enum
from typing import List, Optional, Dict, Any

from pydantic import BaseModel, Field, field_validator
from sqlalchemy import Column, String, Text, DateTime, Boolean, Integer
from sqlalchemy.types import JSON

from app.db_platform import Base

class AgentStatus(str, Enum):
    IDLE = "idle"
    ACTIVE = "active"
    PAUSED = "paused"
    ERROR = "error"

class ReportStatus(str, Enum):
    PENDING = "pending"
    GENERATING = "generating"
    COMPLETED = "completed"
    FAILED = "failed"
    READ = "read"
    IMPORTANT = "important"
    ARCHIVED = "archived"

class AgentDB(Base):
    __tablename__ = "agents"
    id: str = Column(String, primary_key=True, default=lambda: str(uuid.uuid4()), index=True)
    name: str = Column(String, nullable=False, index=True)
    description: Optional[str] = Column(Text)
    keywords: List[str] = Column(JSON, nullable=False, default=lambda: [])
    monitoring_targets: List[str] = Column(JSON, nullable=False, default=lambda: [])
    reporting_frequency: str = Column(String, default="daily")
    status: str = Column(String, default=AgentStatus.IDLE.value)
    last_run_at: Optional[datetime.datetime] = Column(DateTime(timezone=True), nullable=True)
    next_scheduled_run_at: Optional[datetime.datetime] = Column(DateTime(timezone=True), nullable=True)
    created_at: datetime.datetime = Column(DateTime(timezone=True), default=lambda: datetime.datetime.now(datetime.timezone.utc))
    updated_at: datetime.datetime = Column(DateTime(timezone=True), default=lambda: datetime.datetime.now(datetime.timezone.utc), onupdate=lambda: datetime.datetime.now(datetime.timezone.utc))

class ReportDB(Base):
    __tablename__ = "reports"
    id: str = Column(String, primary_key=True, default=lambda: str(uuid.uuid4()), index=True)
    agent_id: str = Column(String, index=True)
    title: str = Column(String, nullable=False)
    summary: Optional[str] = Column(Text)
    content_json: Optional[Dict[str, Any]] = Column(JSON, nullable=True)
    status: str = Column(String, default=ReportStatus.PENDING.value)
    generated_at: datetime.datetime = Column(DateTime(timezone=True), default=lambda: datetime.datetime.now(datetime.timezone.utc))
    data_window_start: Optional[datetime.datetime] = Column(DateTime(timezone=True))
    data_window_end: Optional[datetime.datetime] = Column(DateTime(timezone=True))
    user_annotations: Optional[str] = Column(Text)

class AgentBase(BaseModel):
    name: str = Field(..., min_length=3, max_length=100)
    description: Optional[str] = Field(None, max_length=500)
    keywords: List[str] = Field(..., min_items=1)
    monitoring_targets: List[str] = Field(default_factory=list)
    reporting_frequency: str = Field("daily")

class AgentCreate(AgentBase):
    pass

class AgentUpdate(BaseModel):
    name: Optional[str] = Field(None, min_length=3, max_length=100)
    description: Optional[str] = Field(None, max_length=500)
    keywords: Optional[List[str]] = Field(None, min_items=1)
    monitoring_targets: Optional[List[str]] = None
    reporting_frequency: Optional[str] = None
    status: Optional[AgentStatus] = None

class AgentResponse(AgentBase):
    id: str
    status: AgentStatus
    last_run_at: Optional[datetime.datetime] = None
    next_scheduled_run_at: Optional[datetime.datetime] = None
    created_at: datetime.datetime
    updated_at: datetime.datetime
    model_config = {"from_attributes": True}

class SmartAgentCreateRequest(BaseModel):
    primary_keywords: List[str] = Field(..., min_items=1)

class AgentConfigExportImport(AgentBase):
    status: Optional[AgentStatus] = Field(AgentStatus.IDLE)
    @field_validator('keywords', 'monitoring_targets', mode='before')
    @classmethod
    def ensure_list(cls, v):
        return v if v is not None else []

class ReportBase(BaseModel):
    title: str = Field(..., max_length=200)
    summary: Optional[str] = None
    content_json: Optional[Dict[str, Any]] = Field(default_factory=dict)

class ReportResponse(ReportBase):
    id: str
    agent_id: str
    status: ReportStatus
    generated_at: datetime.datetime
    data_window_start: Optional[datetime.datetime] = None
    data_window_end: Optional[datetime.datetime] = None
    user_annotations: Optional[str] = None
    model_config = {"from_attributes": True}

class ReportUpdate(BaseModel):
    title: Optional[str] = Field(None, max_length=200)
    summary: Optional[str] = None
    user_annotations: Optional[str] = None
    status: Optional[ReportStatus] = None

class ErrorDetailResponse(BaseModel):
    detail: str

class StatusResponse(BaseModel):
    status: str
    message: Optional[str] = None

class HealthStatus(BaseModel):
    app_name: str
    version: str
    status: str = "healthy"
    timestamp: datetime.datetime

class CapabilityDetail(BaseModel):
    method: str
    path: str
    summary: str

class SystemCapabilitiesResponse(BaseModel):
    app_name: str
    version: str
    description: str
    capabilities: List[CapabilityDetail]
    agent_config_schema_info: str = "Agent configurations can be imported/exported as JSON (see AgentConfigExportImport model in API docs)."
