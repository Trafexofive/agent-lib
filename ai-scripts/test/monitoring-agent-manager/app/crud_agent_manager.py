from typing import List, Dict, Optional
import datetime
import uuid
from .models_agent_manager import MonitoringAgent, MonitoringAgentCreate, MonitoringAgentUpdate, Target, TargetCreate

# In-memory storage
_agents_db: Dict[str, MonitoringAgent] = {}

def _transform_targets_create_to_model(target_creates: List[TargetCreate]) -> List[Target]:
    return [Target(**tc.model_dump()) for tc in target_creates]

def create_agent(agent_create: MonitoringAgentCreate) -> MonitoringAgent:
    agent_id = str(uuid.uuid4())
    now = datetime.datetime.now(datetime.timezone.utc)
    
    # Transform TargetCreate to Target for storage
    targets_model = _transform_targets_create_to_model(agent_create.targets)
    
    agent = MonitoringAgent(
        id=agent_id,
        **agent_create.model_dump(exclude={'targets'}), # Exclude targets from base dump
        targets=targets_model, # Add transformed targets
        createdAt=now,
        updatedAt=now
    )
    _agents_db[agent_id] = agent
    return agent

def get_agent(agent_id: str) -> Optional[MonitoringAgent]:
    return _agents_db.get(agent_id)

def get_agents(skip: int = 0, limit: int = 100) -> List[MonitoringAgent]:
    return list(_agents_db.values())[skip: skip + limit]

def update_agent(agent_id: str, agent_update: MonitoringAgentUpdate) -> Optional[MonitoringAgent]:
    agent = _agents_db.get(agent_id)
    if not agent:
        return None

    update_data = agent_update.model_dump(exclude_unset=True)
    
    # Handle nested targets update if provided
    if 'targets' in update_data and update_data['targets'] is not None:
        agent.targets = _transform_targets_create_to_model(agent_update.targets)
        del update_data['targets'] # Remove from dict so it's not overwritten by simple update below

    for key, value in update_data.items():
        setattr(agent, key, value)
    
    agent.updatedAt = datetime.datetime.now(datetime.timezone.utc)
    _agents_db[agent_id] = agent
    return agent

def delete_agent(agent_id: str) -> Optional[MonitoringAgent]:
    return _agents_db.pop(agent_id, None)
