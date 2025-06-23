import logging
from typing import List, Optional

from fastapi import APIRouter, HTTPException, Depends, Query
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.future import select
from sqlalchemy import delete

from app.db_platform import get_db
from app.models_platform import ReportDB, ReportResponse, ReportUpdate, ErrorDetailResponse, StatusResponse, ReportStatus

logger = logging.getLogger(__name__)
router = APIRouter()

@router.get("/", response_model=List[ReportResponse], summary="List reports")
async def list_reports_endpoint(
    agent_id: Optional[str] = Query(None, description="Filter by agent ID."),
    skip: int = Query(0, ge=0),
    limit: int = Query(100, ge=1, le=1000),
    db: AsyncSession = Depends(get_db)
):
    stmt = select(ReportDB).order_by(ReportDB.generated_at.desc())
    if agent_id:
        stmt = stmt.where(ReportDB.agent_id == agent_id)
    stmt = stmt.offset(skip).limit(limit)
    result = await db.execute(stmt)
    reports = result.scalars().all()
    return [ReportResponse.model_validate(report) for report in reports]

@router.get("/{report_id}", response_model=ReportResponse, summary="Get report by ID")
async def get_report_endpoint(report_id: str, db: AsyncSession = Depends(get_db)):
    db_report = await db.get(ReportDB, report_id)
    if db_report is None:
        raise HTTPException(status_code=404, detail="Report not found")
    return ReportResponse.model_validate(db_report)

@router.put("/{report_id}", response_model=ReportResponse, summary="Update report (annotations, status)")
async def update_report_endpoint(report_id: str, report_update: ReportUpdate, db: AsyncSession = Depends(get_db)):
    db_report = await db.get(ReportDB, report_id)
    if db_report is None:
        raise HTTPException(status_code=404, detail="Report not found")
    update_data = report_update.model_dump(exclude_unset=True)
    for key, value in update_data.items():
        if key == 'status' and value is not None:
            setattr(db_report, key, value.value)
        else:
            setattr(db_report, key, value)
    try:
        await db.commit()
        await db.refresh(db_report)
        logger.info(f"Updated report ID: {report_id}")
        return ReportResponse.model_validate(db_report)
    except Exception as e:
        await db.rollback()
        logger.error(f"Failed to update report ID {report_id}: {e}", exc_info=True)
        raise HTTPException(status_code=400, detail=str(e))

@router.delete("/{report_id}", response_model=StatusResponse, summary="Delete report")
async def delete_report_endpoint(report_id: str, db: AsyncSession = Depends(get_db)):
    db_report = await db.get(ReportDB, report_id)
    if db_report is None:
        raise HTTPException(status_code=404, detail="Report not found")
    await db.delete(db_report)
    await db.commit()
    logger.info(f"Deleted report ID: {report_id}")
    return StatusResponse(status="success", message=f"Report {report_id} deleted.")
