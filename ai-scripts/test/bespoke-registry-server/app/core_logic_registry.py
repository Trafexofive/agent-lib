import logging
import os
import uuid
import hashlib
from pathlib import Path
from typing import Optional, Tuple

import aiofiles
from fastapi import UploadFile

from app.config_registry import settings
from app.models_registry import AssetMetadataDB, GeminiAnalysisResult
from app.services import gemini_service # Corrected import path
from app.storage_registry import save_asset_metadata, get_asset_metadata_by_id

logger = logging.getLogger(__name__)

async def process_uploaded_asset(
    file: UploadFile,
    user_description: Optional[str] = None
) -> AssetMetadataDB:
    asset_id = str(uuid.uuid4())
    # Use a unique name for storage to prevent collisions and handle non-ASCII names
    _, file_extension = os.path.splitext(file.filename)
    stored_filename = f"{asset_id}{file_extension if file_extension else '.bin'}"
    stored_file_path = Path(settings.UPLOADED_ASSETS_DIR_CONTAINER) / stored_filename

    logger.info(f"Processing upload: original='{file.filename}', storing as='{stored_filename}', id='{asset_id}'")

    file_content_for_hash = b''
    try:
        # Save the file asynchronously
        async with aiofiles.open(stored_file_path, 'wb') as out_file:
            content_chunk = await file.read(1024*1024) # Read in chunks (e.g., 1MB)
            while content_chunk:
                await out_file.write(content_chunk)
                file_content_for_hash += content_chunk # Accumulate for hashing (be mindful of large files)
                content_chunk = await file.read(1024*1024)
        logger.info(f"File saved to: {stored_file_path}")

        file_size = await aiofiles.os.path.getsize(stored_file_path)

        # For simulation, use a hash of the content. For real Gemini, you might send content or URL.
        content_hash = hashlib.sha256(file_content_for_hash).hexdigest()

        # (SIMULATED) Call Gemini service
        gemini_analysis: GeminiAnalysisResult = await gemini_service.analyze_asset_with_gemini(
            file_content_hash=content_hash,
            original_filename=file.filename,
            user_description=user_description
        )
        logger.info(f"Gemini (simulated) analysis complete for asset {asset_id}: {gemini_analysis.model_dump()}")

        asset_metadata = AssetMetadataDB(
            id=asset_id,
            original_filename=file.filename,
            stored_filename=stored_filename,
            content_type=file.content_type,
            size_bytes=file_size,
            user_description=user_description,
            auto_tags=gemini_analysis.auto_tags,
            quality_rating=gemini_analysis.quality_rating,
            suggested_name=gemini_analysis.suggested_name
            # upload_timestamp is handled by SQLAlchemy default
        )

        saved_metadata = await save_asset_metadata(asset_metadata)
        return saved_metadata

    except Exception as e:
        logger.error(f"Error processing uploaded asset {file.filename}: {e}", exc_info=True)
        # Attempt to clean up partially saved file if it exists
        if await aiofiles.os.path.exists(stored_file_path):
            try:
                await aios.remove(stored_file_path)
                logger.info(f"Cleaned up partially saved file: {stored_file_path}")
            except Exception as cleanup_exc:
                logger.error(f"Error during cleanup of {stored_file_path}: {cleanup_exc}")
        raise # Re-raise the original exception to be handled by API layer

async def get_asset_file_path_and_original_name(asset_id: str) -> Optional[Tuple[Path, str]]:
    metadata = await get_asset_metadata_by_id(asset_id)
    if metadata and metadata.stored_filename:
        file_path = Path(settings.UPLOADED_ASSETS_DIR_CONTAINER) / metadata.stored_filename
        if await aiofiles.os.path.exists(file_path):
            return file_path, metadata.original_filename
        else:
            logger.warning(f"Asset file {metadata.stored_filename} not found on disk for ID {asset_id}")
    return None, None
