import logging
import asyncio
import datetime
from typing import cast, Optional

from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.future import select

from app.db_platform import get_db
from app.models_platform import AgentDB, AgentStatus, ReportDB, ReportStatus
from app.services_platform.information_gatherer_service import gather_information_for_agent, compile_findings_into_report_content

logger = logging.getLogger(__name__)

async def trigger_agent_run(agent_id: str):
    logger.info(f"SCHEDULER (SIMULATED): Triggering run for agent ID: {agent_id}")
    db_session_gen = get_db()
    db: AsyncSession = await anext(db_session_gen) # type: ignore
    agent: Optional[AgentDB] = None
    new_report: Optional[ReportDB] = None
    try:
        stmt = select(AgentDB).where(AgentDB.id == agent_id)
        result = await db.execute(stmt)
        agent = result.scalar_one_or_none()

        if not agent:
            logger.error(f"Agent {agent_id} not found for scheduled run.")
            return

        if agent.status == AgentStatus.PAUSED.value:
            logger.info(f"Agent {agent_id} ({agent.name}) is paused. Skipping run.")
            return
        
        current_run_time = datetime.datetime.now(datetime.timezone.utc)
        agent.status = AgentStatus.ACTIVE.value
        previous_last_run = agent.last_run_at or (current_run_time - datetime.timedelta(days=1))
        agent.last_run_at = current_run_time
        if agent.reporting_frequency == "daily":
            agent.next_scheduled_run_at = current_run_time + datetime.timedelta(days=1)
        elif "PT1H" in agent.reporting_frequency:
             agent.next_scheduled_run_at = current_run_time + datetime.timedelta(hours=1)
        else:
            agent.next_scheduled_run_at = current_run_time + datetime.timedelta(days=1)
        db.add(agent)

        report_title = f"Report for {agent.name} - {current_run_time.strftime('%Y-%m-%d %H:%M UTC')}"
        new_report = ReportDB(
            agent_id=agent.id,
            title=report_title,
            status=ReportStatus.GENERATING.value,
            data_window_start=previous_last_run
        )
        db.add(new_report)
        await db.commit()
        await db.refresh(agent)
        await db.refresh(new_report)
        logger.info(f"Created initial report ID: {new_report.id} for agent {agent.name}")

        findings = await gather_information_for_agent(agent)
        report_content = await compile_findings_into_report_content(agent.name, findings, previous_last_run)
        
        new_report.content_json = report_content
        new_report.status = ReportStatus.COMPLETED.value
        new_report.summary = f"Simulated report with {len(findings)} items. Gathered since {previous_last_run.isoformat()}."
        new_report.data_window_end = current_run_time
        db.add(new_report)

        agent.status = AgentStatus.IDLE.value
        db.add(agent)
        await db.commit()
        logger.info(f"AGENT RUN & REPORT (SIMULATED) COMPLETED for agent {agent.id} ({agent.name}). Report ID: {new_report.id}")

    except Exception as e:
        logger.error(f"Error during scheduled run for agent {agent_id}: {e}", exc_info=True)
        if agent:
            agent.status = AgentStatus.ERROR.value
            db.add(agent)
        if new_report:
            new_report.status = ReportStatus.FAILED.value
            db.add(new_report)
        await db.commit()
    finally:
        await db.close()

async def manual_trigger_all_agents_for_testing():
    logger.info("MANUAL TRIGGER: Attempting to run all non-paused agents.")
    db_session_gen = get_db()
    db: AsyncSession = await anext(db_session_gen) # type: ignore
    try:
        stmt = select(AgentDB).where(AgentDB.status != AgentStatus.PAUSED.value)
        result = await db.execute(stmt)
        agents_to_run = result.scalars().all()
        if not agents_to_run:
            logger.info("MANUAL TRIGGER: No active/idle agents found to run.")
            return
        
        for agent_to_run in agents_to_run:
            await trigger_agent_run(cast(str, agent_to_run.id))
        logger.info(f"MANUAL TRIGGER: Completed simulated runs for {len(agents_to_run)} agents.")
    finally:
        await db.close()
