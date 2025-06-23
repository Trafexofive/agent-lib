import logging
from typing import List, Optional
from pathlib import Path

from fastapi import APIRouter, HTTPException, UploadFile, File, Form, Query
from fastapi.responses import FileResponse

from app.models_registry import AssetMetadataResponse, ErrorDetailResponse
from app.core_logic_registry import process_uploaded_asset, get_asset_file_path_and_original_name
from app.storage_registry import list_all_asset_metadata, get_asset_metadata_by_id, delete_asset_metadata_and_file

logger = logging.getLogger(__name__)
router = APIRouter()

@router.post("/assets", 
            response_model=AssetMetadataResponse,
            status_code=201,
            summary="Upload a new asset and get Gemini-enhanced metadata (simulated)",
            responses={400: {"model": ErrorDetailResponse}, 500: {"model": ErrorDetailResponse}})
async def upload_asset_endpoint(
    file: UploadFile = File(..., description="The asset file to upload."),
    user_description: Optional[str] = Form(None, description="Optional user-provided description for the asset.")
):
    if not file.filename:
        raise HTTPException(status_code=400, detail="Uploaded file must have a filename.")
    try:
        asset_metadata_db = await process_uploaded_asset(file, user_description)
        # Convert DB model to Pydantic response model
        response_data = AssetMetadataResponse(
            **asset_metadata_db.to_dict(), 
            download_url=f"/registry/assets/{asset_metadata_db.id}/download"
        )
        return response_data
    except Exception as e:
        logger.error(f"Asset upload failed for {file.filename}: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Failed to process asset: {str(e)}")

@router.get("/assets", 
            response_model=List[AssetMetadataResponse],
            summary="List all registered assets with their metadata",
            responses={500: {"model": ErrorDetailResponse}})
async def list_assets_endpoint(skip: int = Query(0, ge=0), limit: int = Query(100, ge=1, le=1000)):
    try:
        db_assets = await list_all_asset_metadata(skip=skip, limit=limit)
        response_list = [
            AssetMetadataResponse(
                **asset.to_dict(), 
                download_url=f"/registry/assets/{asset.id}/download"
            ) for asset in db_assets
        ]
        return response_list
    except Exception as e:
        logger.error(f"Failed to list assets: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Failed to retrieve asset list: {str(e)}")

@router.get("/assets/{asset_id}", 
            response_model=AssetMetadataResponse,
            summary="Get metadata for a specific asset",
            responses={404: {"model": ErrorDetailResponse}, 500: {"model": ErrorDetailResponse}})
async def get_asset_metadata_endpoint(asset_id: str):
    try:
        asset_metadata_db = await get_asset_metadata_by_id(asset_id)
        if not asset_metadata_db:
            raise HTTPException(status_code=404, detail=f"Asset with ID '{asset_id}' not found.")
        response_data = AssetMetadataResponse(
            **asset_metadata_db.to_dict(), 
            download_url=f"/registry/assets/{asset_metadata_db.id}/download"
        )
        return response_data
    except HTTPException: # Re-raise specific HTTPExceptions
        raise
    except Exception as e:
        logger.error(f"Failed to get metadata for asset {asset_id}: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Failed to retrieve asset metadata: {str(e)}")

@router.get("/assets/{asset_id}/download",
            response_class=FileResponse,
            summary="Download an asset file",
            responses={
                404: {"model": ErrorDetailResponse, "description": "Asset or file not found"},
                500: {"model": ErrorDetailResponse, "description": "Error accessing asset file"}
            })
async def download_asset_file_endpoint(asset_id: str):
    try:
        file_path, original_filename = await get_asset_file_path_and_original_name(asset_id)
        if not file_path or not original_filename:
            raise HTTPException(status_code=404, detail=f"Asset file for ID '{asset_id}' not found or metadata incomplete.")
        
        return FileResponse(path=file_path, filename=original_filename, media_type='application/octet-stream')
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to download asset {asset_id}: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Failed to download asset file: {str(e)}")

@router.delete("/assets/{asset_id}",
               status_code=200, # Or 204 if no content is returned
               response_model=dict, # e.g. {"message": "Asset deleted"}
               summary="Delete an asset (metadata and file)",
               responses={404: {"model": ErrorDetailResponse}, 500: {"model": ErrorDetailResponse}})
async def delete_asset_endpoint(asset_id: str):
    try:
        deleted = await delete_asset_metadata_and_file(asset_id)
        if not deleted:
            raise HTTPException(status_code=404, detail=f"Asset with ID '{asset_id}' not found or already deleted.")
        return {"id": asset_id, "status": "deleted", "message": "Asset and its metadata successfully deleted."}
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to delete asset {asset_id}: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=f"Failed to delete asset: {str(e)}")
