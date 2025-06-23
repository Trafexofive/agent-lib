import logging
import json
from typing import List, Optional, cast, Any
from enum import Enum

from fastapi import APIRouter, HTTPException, Depends, UploadFile, File, Body, Query
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.future import select
from sqlalchemy import delete

from app.db_platform import get_db
from app.models_platform import (
    AgentDB, AgentCreate, AgentUpdate, AgentResponse, 
    SmartAgentCreateRequest, AgentConfigExportImport, ErrorDetailResponse, StatusResponse,
    AgentStatus, ReportDB
)
from app.services_platform import agent_creation_service, scheduling_service

logger = logging.getLogger(__name__)
router = APIRouter()

@router.post("/smart-create", response_model=AgentResponse, status_code=201, summary="Smartly create an agent (Simulated)")
async def smart_create_agent_endpoint(request: SmartAgentCreateRequest, db: AsyncSession = Depends(get_db)):
    try:
        agent_create_data: AgentCreate = await agent_creation_service.generate_smart_agent_config(request)
        db_agent = AgentDB(**agent_create_data.model_dump())
        db.add(db_agent)
        await db.commit()
        await db.refresh(db_agent)
        logger.info(f"Smartly created agent '{db_agent.name}' (ID: {db_agent.id})")
        return AgentResponse.model_validate(db_agent)
    except Exception as e:
        await db.rollback()
        logger.error(f"Smart agent creation failed: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/", response_model=AgentResponse, status_code=201, summary="Manually create an agent")
async def create_agent_endpoint(agent: AgentCreate, db: AsyncSession = Depends(get_db)):
    db_agent = AgentDB(**agent.model_dump())
    try:
        db.add(db_agent)
        await db.commit()
        await db.refresh(db_agent)
        logger.info(f"Created agent '{db_agent.name}' (ID: {db_agent.id})")
        return AgentResponse.model_validate(db_agent)
    except Exception as e:
        await db.rollback()
        logger.error(f"Failed to create agent '{agent.name}': {e}", exc_info=True)
        raise HTTPException(status_code=400, detail=str(e))

@router.get("/", response_model=List[AgentResponse], summary="List agents")
async def list_agents_endpoint(skip: int = Query(0, ge=0), limit: int = Query(100, ge=1, le=1000), db: AsyncSession = Depends(get_db)):
    result = await db.execute(select(AgentDB).order_by(AgentDB.created_at.desc()).offset(skip).limit(limit))
    agents = result.scalars().all()
    return [AgentResponse.model_validate(agent) for agent in agents]

@router.get("/{agent_id}", response_model=AgentResponse, summary="Get agent by ID")
async def get_agent_endpoint(agent_id: str, db: AsyncSession = Depends(get_db)):
    db_agent = await db.get(AgentDB, agent_id)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    return AgentResponse.model_validate(db_agent)

@router.put("/{agent_id}", response_model=AgentResponse, summary="Update agent")
async def update_agent_endpoint(agent_id: str, agent_update: AgentUpdate, db: AsyncSession = Depends(get_db)):
    db_agent = await db.get(AgentDB, agent_id)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    update_data = agent_update.model_dump(exclude_unset=True)
    for key, value in update_data.items():
        if key == 'status' and value is not None:
            setattr(db_agent, key, value.value) 
        else:
            setattr(db_agent, key, value)
    try:
        await db.commit()
        await db.refresh(db_agent)
        logger.info(f"Updated agent ID: {agent_id}")
        return AgentResponse.model_validate(db_agent)
    except Exception as e:
        await db.rollback()
        logger.error(f"Failed to update agent ID {agent_id}: {e}", exc_info=True)
        raise HTTPException(status_code=400, detail=str(e))

@router.delete("/{agent_id}", response_model=StatusResponse, summary="Delete agent")
async def delete_agent_endpoint(agent_id: str, delete_reports: bool = Query(False), db: AsyncSession = Depends(get_db)):
    db_agent = await db.get(AgentDB, agent_id)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    if delete_reports:
        await db.execute(delete(ReportDB).where(ReportDB.agent_id == agent_id))
        logger.info(f"Deleted reports for agent ID: {agent_id}")
    await db.delete(db_agent)
    await db.commit()
    logger.info(f"Deleted agent ID: {agent_id}")
    return StatusResponse(status="success", message=f"Agent {agent_id} deleted.")

@router.post("/{agent_id}/pause", response_model=AgentResponse, summary="Pause agent")
async def pause_agent_endpoint(agent_id: str, db: AsyncSession = Depends(get_db)):
    return await update_agent_endpoint(agent_id, AgentUpdate(status=AgentStatus.PAUSED), db)

@router.post("/{agent_id}/resume", response_model=AgentResponse, summary="Resume agent")
async def resume_agent_endpoint(agent_id: str, db: AsyncSession = Depends(get_db)):
    return await update_agent_endpoint(agent_id, AgentUpdate(status=AgentStatus.IDLE), db)

@router.post("/import", response_model=List[AgentResponse], summary="Import agents")
async def import_agents_endpoint(agents_config_list: List[AgentConfigExportImport] = Body(...), db: AsyncSession = Depends(get_db)):
    created_agents = []
    errors = []
    for i, agent_config in enumerate(agents_config_list):
        try:
            agent_create_data = AgentCreate(**agent_config.model_dump(exclude_defaults=True, exclude_none=True))
            db_agent = AgentDB(**agent_create_data.model_dump())
            if agent_config.status:
                 db_agent.status = agent_config.status.value
            db.add(db_agent)
            await db.commit()
            await db.refresh(db_agent)
            created_agents.append(AgentResponse.model_validate(db_agent))
        except Exception as e:
            await db.rollback()
            error_detail = f"Error importing agent index {i} (name: {agent_config.name or 'N/A'}): {str(e)}"
            logger.error(error_detail, exc_info=True)
            errors.append(error_detail)
    if errors:
        raise HTTPException(status_code=400, detail=f"Import failed for some agents: {'; '.join(errors)}")
    return created_agents

@router.get("/{agent_id}/export", response_model=AgentConfigExportImport, summary="Export agent config")
async def export_agent_endpoint(agent_id: str, db: AsyncSession = Depends(get_db)):
    db_agent = await db.get(AgentDB, agent_id)
    if db_agent is None:
        raise HTTPException(status_code=404, detail="Agent not found")
    agent_dict = {c.name: getattr(db_agent, c.name) for c in db_agent.__table__.columns}
    if 'status' in agent_dict and isinstance(agent_dict['status'], str):
        try: agent_dict['status'] = AgentStatus(agent_dict['status'])
        except ValueError: agent_dict['status'] = AgentStatus.IDLE
    return AgentConfigExportImport(**agent_dict)

@router.post("/{agent_id}/trigger-run", response_model=StatusResponse, summary="Manually trigger agent run (Simulated)")
async def trigger_agent_run_endpoint(agent_id: str):
    logger.info(f"API trigger for agent run: {agent_id}")
    try:
        await scheduling_service.trigger_agent_run(agent_id)
        return StatusResponse(status="success", message=f"Simulated run triggered for agent {agent_id}.")
    except Exception as e:
        logger.error(f"API trigger failed for agent {agent_id}: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/trigger-all-runs-test", response_model=StatusResponse, summary="Trigger all non-paused agents (Simulated)")
async def trigger_all_agents_test_endpoint():
    logger.info(f"API trigger for all agent runs.")
    try:
        await scheduling_service.manual_trigger_all_agents_for_testing()
        return StatusResponse(status="success", message=f"Simulated runs dispatched for non-paused agents.")
    except Exception as e:
        logger.error(f"API trigger for all agents failed: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))
