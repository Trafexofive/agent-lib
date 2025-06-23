from pydantic import BaseModel, Field, field_validator
from typing import List, Optional
import datetime
import uuid

class TargetBase(BaseModel):
    id: str
    description: str
    sourceType: str = Field(..., examples=["WEB_SEARCH"])
    sourceValue: str
    informationToExtract: str

class TargetCreate(TargetBase):
    pass # Inherits all fields, id might be optional if server generates

class Target(TargetBase):
    pass # No changes from base for now

class MonitoringAgentBase(BaseModel):
    name: str
    description: str
    targets: List[Target]
    reportFrequency: str = Field(..., examples=["Daily", "Weekly", "Monthly"])
    status: str = Field("active", examples=["active", "paused", "archived"])
    lastRunStatus: Optional[str] = Field("pending", examples=["pending", "success", "failed"])

class MonitoringAgentCreate(MonitoringAgentBase):
    # id, createdAt, updatedAt will be server-generated
    targets: List[TargetCreate] # Use TargetCreate for creating new agents
    # Optionally allow client to suggest an ID, or always generate server-side
    # id: Optional[str] = None 

class MonitoringAgentUpdate(BaseModel):
    name: Optional[str] = None
    description: Optional[str] = None
    targets: Optional[List[TargetCreate]] = None
    reportFrequency: Optional[str] = None
    status: Optional[str] = None
    lastRunStatus: Optional[str] = None

class MonitoringAgent(MonitoringAgentBase):
    id: str = Field(default_factory=lambda: str(uuid.uuid4()))
    createdAt: datetime.datetime = Field(default_factory=datetime.datetime.now)
    updatedAt: datetime.datetime = Field(default_factory=datetime.datetime.now)

    class Config:
        from_attributes = True # for ORM mode if using SQLAlchemy models later

class SystemHealth(BaseModel):
    app_name: str
    version: str
    status: str = "healthy"
    timestamp: datetime.datetime

class SystemCapabilities(BaseModel):
    app_name: str
    version: str
    description: str
    supported_operations: List[str]
    endpoints: List[dict]
    # potential_ai_integration_points: List[str]
