import json
import os
from typing import List, Dict, Any
from fastapi import FastAPI, Depends, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session

import db_crud, db_models, pydantic_models
from database import SessionLocal, engine

# Create tables if they don't exist (Alembic is preferred for production)
# db_models.Base.metadata.create_all(bind=engine)

app = FastAPI(
    title="Agent Factory Relic",
    version="0.1.0"
)

# CORS middleware for frontend access
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allow all origins for simplicity in homelab
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "DELETE"],
    allow_headers=["*"],
)

# Dependency to get DB session
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

@app.on_event("startup")
def on_startup():
    # Check if the database has any agents. If not, load the default.
    db = SessionLocal()
    if db_crud.get_agents(db, skip=0, limit=1) == []:
        print("No agents found in DB. Loading default agent template...")
        try:
            with open("default_agent_template.json", "r") as f:
                default_agent_data = json.load(f)
                agent_create = pydantic_models.AgentCreate(definition=default_agent_data)
                db_crud.create_agent(db=db, agent=agent_create)
                print("Default agent template loaded successfully.")
        except Exception as e:
            print(f"Error loading default agent template: {e}")
    db.close()

@app.get("/context", response_model=pydantic_models.RelicContext)
def get_relic_context():
    """Provides self-descriptive metadata about the relic for agent discovery."""
    return {
        "relic_name": "agent_factory",
        "version": "0.1.0",
        "description": "A relic to create, manage, and store structured definitions (blueprints) for AI agents. It serves as the foundational component for an agent manufacturing and deployment system.",
        "system_prompt_fragment": "To interact with the Agent Factory, you should use its REST API. You can list all agent blueprints with a GET to /api/v1/agents. To create a new agent blueprint, POST a valid agent JSON object to /api/v1/agents. The required schema for an agent object can be retrieved by examining an existing agent or the default template.",
        "capabilities": [
            "Manages a database of agent definitions.",
            "Provides CRUD API for agent blueprints.",
            "Serves a lightweight frontend for human interaction."
        ],
        "status_notes": [
            "Operational. Database backend is PostgreSQL."
        ]
    }

@app.post("/api/v1/agents", response_model=pydantic_models.Agent)
def create_agent(agent: pydantic_models.AgentCreate, db: Session = Depends(get_db)):
    """Creates a new Agent Definition from a JSON body."""
    return db_crud.create_agent(db=db, agent=agent)

@app.get("/api/v1/agents", response_model=List[pydantic_models.Agent])
def read_agents(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    """Retrieves a list of all stored Agent Definitions."""
    agents = db_crud.get_agents(db, skip=skip, limit=limit)
    return agents

@app.get("/api/v1/agents/{agent_id}", response_model=pydantic_models.Agent)
def read_agent(agent_id: int, db: Session = Depends(get_db)):
    """Retrieves a specific Agent Definition by its ID."""
    db_agent = db_crud.get_agent(db, agent_id=agent_id)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    return db_agent

@app.put("/api/v1/agents/{agent_id}", response_model=pydantic_models.Agent)
def update_agent(agent_id: int, agent: pydantic_models.AgentCreate, db: Session = Depends(get_db)):
    """Updates an existing Agent Definition."""
    db_agent = db_crud.update_agent(db, agent_id=agent_id, agent_definition=agent.definition)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    return db_agent

@app.delete("/api/v1/agents/{agent_id}", response_model=pydantic_models.Agent)
def delete_agent(agent_id: int, db: Session = Depends(get_db)):
    """Deletes an Agent Definition."""
    db_agent = db_crud.delete_agent(db, agent_id=agent_id)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    return db_agent
