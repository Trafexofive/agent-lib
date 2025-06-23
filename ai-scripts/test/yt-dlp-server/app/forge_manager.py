import os
import tarfile
import shutil
import aiofiles
import aiofiles.os as aios # For async os operations
import asyncio
from typing import Tuple

from app.models_forge import RelicPlan
from app.config_forge import settings # Updated import name
import logging

logger = logging.getLogger(__name__)

class ForgeManager:
    # No need for __init__ if it only sets self.settings from global settings

    async def forge_relic(self, relic_id: str, plan: RelicPlan) -> Tuple[str, str]:
        # Base directory for all content of this specific relic instance before packaging
        relic_content_sourcedir = os.path.join(settings.storage_path, "forged_relics", relic_id + "_source")
        # Ensure clean slate for source directory
        if await aios.path.exists(relic_content_sourcedir):
            await asyncio.to_thread(shutil.rmtree, relic_content_sourcedir)
        await aios.makedirs(relic_content_sourcedir, exist_ok=True)
        logger.info(f"Created source directory for relic {relic_id} at {relic_content_sourcedir}")

        for artifact in plan.artifacts:
            # Paths in artifact.path are relative to the root of the relic being forged
            # Sanitize artifact.path to prevent escaping relic_content_sourcedir (already handled by Pydantic model validator)
            target_artifact_path = os.path.join(relic_content_sourcedir, artifact['path'])
            
            # Ensure parent directory for the artifact exists
            artifact_dir = os.path.dirname(target_artifact_path)
            if not await aios.path.exists(artifact_dir):
                await aios.makedirs(artifact_dir, exist_ok=True)
            
            async with aiofiles.open(target_artifact_path, "w", encoding='utf-8') as f:
                await f.write(artifact['content'])
            logger.debug(f"Wrote artifact '{artifact['path']}' for relic {relic_id}")

        # Define package name and full path
        package_filename = f"{relic_id}_{plan.relic_name.replace(' ', '_')}_{plan.version}.tar.gz"
        full_package_path = os.path.join(settings.storage_path, "forged_relics", package_filename)

        # Create tar.gz package asynchronously
        def _create_tarball():
            with tarfile.open(full_package_path, "w:gz") as tar:
                # Add the contents of relic_content_sourcedir to the tarball.
                # arcname='.' means files will be at the root of the tarball, not inside a 'relic_content_sourcedir' folder.
                # Or, to have a root folder in tar: arcname=plan.relic_name
                tar.add(relic_content_sourcedir, arcname=plan.relic_name) 
            logger.info(f"Packaged relic {relic_id} to {full_package_path}")
        
        await asyncio.to_thread(_create_tarball)

        # Clean up the temporary source directory after packaging
        await asyncio.to_thread(shutil.rmtree, relic_content_sourcedir)
        logger.info(f"Cleaned up source directory {relic_content_sourcedir} for relic {relic_id}")

        return package_filename, full_package_path # Return filename and full path
