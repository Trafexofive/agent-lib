from sqlalchemy.orm import Session
import db_models, pydantic_models

def get_agent(db: Session, agent_id: int):
    return db.query(db_models.Agent).filter(db_models.Agent.id == agent_id).first()

def get_agents(db: Session, skip: int = 0, limit: int = 100):
    return db.query(db_models.Agent).offset(skip).limit(limit).all()

def create_agent(db: Session, agent: pydantic_models.AgentCreate):
    db_agent = db_models.Agent(definition=agent.definition)
    db.add(db_agent)
    db.commit()
    db.refresh(db_agent)
    return db_agent

def update_agent(db: Session, agent_id: int, agent_definition: dict):
    db_agent = get_agent(db, agent_id)
    if db_agent:
        db_agent.definition = agent_definition
        db.commit()
        db.refresh(db_agent)
    return db_agent

def delete_agent(db: Session, agent_id: int):
    db_agent = get_agent(db, agent_id)
    if db_agent:
        db.delete(db_agent)
        db.commit()
    return db_agent
