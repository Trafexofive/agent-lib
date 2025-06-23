import logging
import uuid
import os
import shutil
import tarfile
import datetime
import asyncio
from pathlib import Path

from fastapi import APIRouter, HTTPException, Body, Depends, status
from fastapi.responses import FileResponse
from typing import List

from app.models_forge import (
    FullRelicPlanInput,
    RelicForgedResponse,
    RelicMetadata, 
    RelicMetadataInDB,
    HTTPErrorDetail
)
from app.config_forge import settings
from app.storage_manager import (
    store_relic_metadata,
    get_relic_metadata_by_id,
    get_all_relics_metadata_from_db,
    delete_relic_from_db_and_fs
)
import aiofiles
import aiofiles.os as aios

logger = logging.getLogger(__name__)
router = APIRouter()

FORGED_RELICS_BASE_PATH = Path(settings.FORGED_RELICS_DIR).resolve()

async def create_directory_if_not_exists(path: Path):
    if not await aios.path.exists(path):
        await aios.makedirs(path, exist_ok=True)
        logger.info(f"Created directory: {path}")
    elif not await aios.path.isdir(path):
        logger.error(f"Path {path} exists but is not a directory.")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Path {path} exists but is not a directory."
        )

@router.post("/relic", 
             response_model=RelicForgedResponse, 
             status_code=status.HTTP_201_CREATED,
             summary="Forge a new relic based on a plan",
             responses={
                 400: {"model": HTTPErrorDetail, "description": "Invalid input plan"},
                 422: {"model": HTTPErrorDetail, "description": "Validation Error"},
                 500: {"model": HTTPErrorDetail, "description": "Internal server error"}
             })
async def forge_relic_endpoint(plan: FullRelicPlanInput = Body(...)):
    relic_id = str(uuid.uuid4())
    relic_name_sanitized = Path(plan.relic_name).name 
    timestamp = datetime.datetime.now(datetime.timezone.utc)

    await create_directory_if_not_exists(FORGED_RELICS_BASE_PATH) 

    relic_instance_path = FORGED_RELICS_BASE_PATH / relic_id
    relic_content_output_path = relic_instance_path / relic_name_sanitized

    package_filename = f"{relic_name_sanitized}-{plan.version}.tar.gz"
    package_full_path = relic_instance_path / package_filename

    try:
        await create_directory_if_not_exists(relic_instance_path)
        await create_directory_if_not_exists(relic_content_output_path)

        if plan.bootstrap_script_content:
            logger.info(f"[Relic: {relic_id}] Bootstrap script provided, content logged (execution is simulated/skipped).")

        for artifact in plan.artifacts:
            artifact_path_segment = Path(artifact.path)
            if artifact_path_segment.is_absolute() or '..' in artifact_path_segment.parts:
                logger.error(f"Invalid artifact path attempt: {artifact.path} for relic {relic_id}")
                raise HTTPException(status_code=400, detail=f"Invalid artifact path: '{artifact.path}'. Must be relative and within project.")
            
            full_artifact_path = (relic_content_output_path / artifact_path_segment).resolve()
            
            if relic_content_output_path.resolve() not in full_artifact_path.parents and full_artifact_path != relic_content_output_path.resolve():
                 logger.error(f"Path traversal attempt detected for artifact: {artifact.path} resolving to {full_artifact_path}")
                 raise HTTPException(status_code=400, detail=f"Invalid artifact path (potential traversal): {artifact.path}")

            await create_directory_if_not_exists(full_artifact_path.parent)
            
            async with aiofiles.open(full_artifact_path, mode='w', encoding='utf-8') as f:
                await f.write(artifact.content)
            logger.debug(f"[Relic: {relic_id}] Created artifact: {full_artifact_path}")

        def create_tarball_sync():
            with tarfile.open(package_full_path, "w:gz") as tar:
                tar.add(str(relic_content_output_path), arcname=relic_name_sanitized)
        
        await asyncio.to_thread(create_tarball_sync)
        logger.info(f"[Relic: {relic_id}] Packaged relic to: {package_full_path}")

        relic_metadata = RelicMetadataInDB(
            id=relic_id,
            relic_name=plan.relic_name,
            version=plan.version,
            package_filename=package_filename,
            package_path=str(package_full_path.relative_to(FORGED_RELICS_BASE_PATH)),
            status="forged",
            timestamp_utc=timestamp,
        )
        await store_relic_metadata(relic_metadata)
        logger.info(f"[Relic: {relic_id}] Stored metadata for {plan.relic_name} v{plan.version}")

        return RelicForgedResponse(
            relic_id=uuid.UUID(relic_id),
            relic_name=plan.relic_name,
            version=plan.version,
            package_filename=package_filename,
            download_url=f"/forge/relics/{relic_id}/download",
            status="forged",
            timestamp=timestamp
        )

    except HTTPException as http_exc:
        logger.error(f"HTTPException during forging relic {relic_id}: {http_exc.detail}")
        raise
    except Exception as e:
        logger.error(f"Error forging relic {relic_id} ({plan.relic_name}): {e}", exc_info=True)
        if await aios.path.exists(relic_instance_path):
            try:
                await asyncio.to_thread(shutil.rmtree, relic_instance_path)
                logger.info(f"Cleaned up partially forged relic directory: {relic_instance_path}")
            except Exception as cleanup_exc:
                logger.error(f"Error during cleanup of {relic_instance_path}: {cleanup_exc}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR, 
            detail=f"Failed to forge relic: {str(e)}"
        )

@router.get("/relics", 
            response_model=List[RelicMetadata],
            summary="List all forged relics and their metadata")
async def list_forged_relics():
    try:
        db_relics = await get_all_relics_metadata_from_db()
        return [
            RelicMetadata(
                relic_id=uuid.UUID(r.id),
                relic_name=r.relic_name,
                version=r.version,
                package_filename=r.package_filename,
                download_url=f"/forge/relics/{r.id}/download",
                status=r.status,
                timestamp=r.timestamp_utc
            ) for r in db_relics
        ]
    except Exception as e:
        logger.error(f"Error listing relics: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=HTTPErrorDetail(message=f"Could not retrieve relic list: {str(e)}").model_dump())

@router.get("/relics/{relic_id}", 
            response_model=RelicMetadata,
            summary="Get metadata for a specific relic",
            responses={404: {"model": HTTPErrorDetail}})
async def get_relic_info(relic_id: str):
    try:
        parsed_uuid = uuid.UUID(relic_id) 
    except ValueError:
        raise HTTPException(status_code=400, detail=HTTPErrorDetail(message=f"Invalid relic_id format: '{relic_id}'. Must be a UUID.").model_dump())
    
    db_relic = await get_relic_metadata_by_id(str(parsed_uuid))
    if not db_relic:
        raise HTTPException(status_code=404, detail=HTTPErrorDetail(message=f"Relic with ID '{relic_id}' not found.").model_dump())
    
    return RelicMetadata(
        relic_id=uuid.UUID(db_relic.id),
        relic_name=db_relic.relic_name,
        version=db_relic.version,
        package_filename=db_relic.package_filename,
        download_url=f"/forge/relics/{db_relic.id}/download",
        status=db_relic.status,
        timestamp=db_relic.timestamp_utc
    )

@router.get("/relics/{relic_id}/download",
            summary="Download a forged relic package (.tar.gz)",
            response_class=FileResponse,
            responses={404: {"model": HTTPErrorDetail}})
async def download_forged_relic(relic_id: str):
    try:
        parsed_uuid = uuid.UUID(relic_id)
    except ValueError:
        raise HTTPException(status_code=400, detail=HTTPErrorDetail(message=f"Invalid relic_id format: '{relic_id}'. Must be a UUID.").model_dump())

    db_relic = await get_relic_metadata_by_id(str(parsed_uuid))
    if not db_relic:
        raise HTTPException(status_code=404, detail=HTTPErrorDetail(message=f"Relic metadata for ID '{relic_id}' not found.").model_dump())

    package_full_path = FORGED_RELICS_BASE_PATH / db_relic.package_path

    if not await aios.path.exists(package_full_path) or not await aios.path.isfile(package_full_path):
        logger.error(f"Relic package file not found at {package_full_path} for relic_id {relic_id}")
        raise HTTPException(status_code=404, detail=HTTPErrorDetail(message=f"Relic package file for ID '{relic_id}' not found on server.").model_dump())
    
    return FileResponse(
        path=str(package_full_path),
        filename=db_relic.package_filename,
        media_type='application/gzip'
    )

@router.delete("/relics/{relic_id}",
               status_code=status.HTTP_204_NO_CONTENT,
               summary="Delete a forged relic (metadata and package)",
               responses={404: {"model": HTTPErrorDetail}})
async def delete_forged_relic(relic_id: str):
    try:
        parsed_uuid = uuid.UUID(relic_id)
    except ValueError:
        raise HTTPException(status_code=400, detail=HTTPErrorDetail(message=f"Invalid relic_id format: '{relic_id}'. Must be a UUID.").model_dump())

    success = await delete_relic_from_db_and_fs(str(parsed_uuid))
    if not success:
        raise HTTPException(status_code=404, detail=HTTPErrorDetail(message=f"Relic with ID '{relic_id}' not found or already deleted.").model_dump())
    return
