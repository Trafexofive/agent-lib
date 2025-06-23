from pydantic import BaseModel
from typing import List, Dict, Any
import datetime

# --- API Request/Response Models ---

class AgentCreate(BaseModel):
    definition: Dict[str, Any]

class Agent(BaseModel):
    id: int
    definition: Dict[str, Any]
    created_at: datetime.datetime
    updated_at: datetime.datetime | None = None

    class Config:
        from_attributes = True

# --- Context Endpoint Model ---

class RelicContext(BaseModel):
    relic_name: str
    version: str
    description: str
    system_prompt_fragment: str
    capabilities: List[str]
    status_notes: List[str]
