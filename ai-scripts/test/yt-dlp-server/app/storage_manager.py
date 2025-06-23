import os
import aiofiles.os as aios # For async os operations
import asyncio
from typing import List, Optional

from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from sqlalchemy import select, delete

from app.models_forge import RelicMetadataDB, Base # Renamed to RelicMetadataDB to distinguish SQLAlchemy model
from app.config_forge import settings # Updated import name
import logging

logger = logging.getLogger(__name__)

class StorageManager:
    def __init__(self):
        db_path = os.path.join(settings.storage_path, "forge_db", "relics.db")
        self.db_url = f"sqlite+aiosqlite:///{db_path}"
        logger.info(f"Database URL configured: {self.db_url}")
        self.engine = create_async_engine(self.db_url, echo=(settings.app_env == "development"))
        self.AsyncSessionFactory = sessionmaker(self.engine, class_=AsyncSession, expire_on_commit=False)

    async def init_db(self):
        async with self.engine.begin() as conn:
            await conn.run_sync(Base.metadata.create_all)
        logger.info("Database tables checked/created.")

    def create_db_metadata_object(self, **kwargs) -> RelicMetadataDB:
        """Helper to create a RelicMetadataDB instance."""
        return RelicMetadataDB(**kwargs)

    async def store_metadata(self, metadata: RelicMetadataDB):
        async with self.AsyncSessionFactory() as session:
            async with session.begin():
                session.add(metadata)
            # Removed explicit commit, session.begin() handles it.
            logger.info(f"Stored metadata for relic: {metadata.relic_id}")

    async def list_relics(self) -> List[RelicMetadataDB]:
        async with self.AsyncSessionFactory() as session:
            result = await session.execute(select(RelicMetadataDB).order_by(RelicMetadataDB.timestamp.desc()))
            return result.scalars().all()
    
    async def get_relic_metadata_by_id(self, relic_id: str) -> Optional[RelicMetadataDB]:
        async with self.AsyncSessionFactory() as session:
            result = await session.execute(select(RelicMetadataDB).where(RelicMetadataDB.relic_id == relic_id))
            return result.scalar_one_or_none()

    async def delete_relic(self, relic_id: str) -> bool:
        metadata = await self.get_relic_metadata_by_id(relic_id)
        if not metadata:
            logger.warning(f"Attempted to delete non-existent relic metadata: {relic_id}")
            return False

        package_path = os.path.join(settings.storage_path, "forged_relics", metadata.package_filename)
        
        deleted_from_db = False
        async with self.AsyncSessionFactory() as session:
            async with session.begin():
                stmt = delete(RelicMetadataDB).where(RelicMetadataDB.relic_id == relic_id)
                result = await session.execute(stmt)
                if result.rowcount > 0:
                    deleted_from_db = True
                    logger.info(f"Deleted relic metadata from DB: {relic_id}")
                else:
                    logger.warning(f"Relic metadata for {relic_id} not found in DB for deletion.")
        
        if deleted_from_db: # Only attempt to delete file if DB record was deleted
            try:
                if await aios.path.exists(package_path):
                    await aios.remove(package_path)
                    logger.info(f"Deleted relic package file: {package_path}")
                else:
                    logger.warning(f"Relic package file not found for deletion: {package_path}")
            except Exception as e_fs:
                logger.error(f"Error deleting relic package file {package_path}: {e_fs}. DB record was already deleted.", exc_info=True)
                # This is a problematic state. Consider how to handle (e.g., orphan cleanup task).
        
        return deleted_from_db
