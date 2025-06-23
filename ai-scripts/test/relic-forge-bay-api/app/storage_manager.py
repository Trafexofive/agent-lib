import logging
import shutil
import asyncio
from pathlib import Path
from typing import List, Optional

from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from sqlalchemy import select, delete as sqlalchemy_delete
from sqlalchemy.exc import SQLAlchemyError

from app.config_forge import settings
from app.models_forge import RelicMetadataInDB, DbBase # Use DbBase from models_forge
import aiofiles.os as aios

logger = logging.getLogger(__name__)

_ASYNC_ENGINE = None
_AsyncSessionFactory = None

FORGED_RELICS_BASE_PATH = Path(settings.FORGED_RELICS_DIR).resolve()
DB_FILE_PATH = Path(settings.FORGE_DATABASE_URL.split('///')[-1]).resolve()

def get_engine():
    global _ASYNC_ENGINE
    if _ASYNC_ENGINE is None:
        logger.info(f"Creating async SQLAlchemy engine for URL: {settings.FORGE_DATABASE_URL}")
        if settings.FORGE_DATABASE_URL.startswith("sqlite"):
            DB_FILE_PATH.parent.mkdir(parents=True, exist_ok=True)
            logger.info(f"Ensured database directory exists: {DB_FILE_PATH.parent}")
        _ASYNC_ENGINE = create_async_engine(settings.FORGE_DATABASE_URL, echo=(settings.LOG_LEVEL.upper() == "DEBUG"))
    return _ASYNC_ENGINE

def get_async_session_factory():
    global _AsyncSessionFactory
    if _AsyncSessionFactory is None:
        engine = get_engine()
        _AsyncSessionFactory = sessionmaker(
            bind=engine,
            class_=AsyncSession,
            expire_on_commit=False
        )
    return _AsyncSessionFactory

async def get_db_session() -> AsyncSession:
    factory = get_async_session_factory()
    async with factory() as session:
        yield session

async def init_db():
    engine = get_engine()
    async with engine.begin() as conn:
        await conn.run_sync(DbBase.metadata.create_all)
    logger.info("Database tables checked/created.")

async def close_db_connection():
    global _ASYNC_ENGINE
    if _ASYNC_ENGINE:
        await _ASYNC_ENGINE.dispose()
        _ASYNC_ENGINE = None
        logger.info("Async SQLAlchemy engine disposed.")

async def store_relic_metadata(relic_data: RelicMetadataInDB) -> RelicMetadataInDB:
    factory = get_async_session_factory()
    async with factory() as session:
        async with session.begin():
            try:
                session.add(relic_data)
            except SQLAlchemyError as e:
                logger.error(f"Database error preparing to store relic metadata {relic_data.id}: {e}", exc_info=True)
                raise
        await session.refresh(relic_data)
        logger.info(f"Stored metadata for relic: {relic_data.id}")
        return relic_data

async def get_relic_metadata_by_id(relic_id: str) -> Optional[RelicMetadataInDB]:
    factory = get_async_session_factory()
    async with factory() as session:
        try:
            stmt = select(RelicMetadataInDB).where(RelicMetadataInDB.id == relic_id)
            result = await session.execute(stmt)
            relic = result.scalar_one_or_none()
            if relic:
                logger.debug(f"Retrieved metadata for relic: {relic_id}")
            else:
                logger.debug(f"No metadata found for relic: {relic_id}")
            return relic
        except SQLAlchemyError as e:
            logger.error(f"Database error retrieving relic metadata {relic_id}: {e}", exc_info=True)
            raise

async def get_all_relics_metadata_from_db() -> List[RelicMetadataInDB]:
    factory = get_async_session_factory()
    async with factory() as session:
        try:
            stmt = select(RelicMetadataInDB).order_by(RelicMetadataInDB.timestamp_utc.desc())
            result = await session.execute(stmt)
            relics = result.scalars().all()
            logger.debug(f"Retrieved {len(relics)} relic metadata records.")
            return list(relics)
        except SQLAlchemyError as e:
            logger.error(f"Database error retrieving all relic metadata: {e}", exc_info=True)
            raise

async def delete_relic_from_db_and_fs(relic_id: str) -> bool:
    factory = get_async_session_factory()
    async with factory() as session:
        relic_to_delete = None
        try:
            async with session.begin():
                stmt_select = select(RelicMetadataInDB).where(RelicMetadataInDB.id == relic_id)
                result = await session.execute(stmt_select)
                relic_to_delete = result.scalar_one_or_none()

                if not relic_to_delete:
                    logger.warning(f"Attempted to delete non-existent relic (ID: {relic_id}) from DB. Not found.")
                    return False

                stmt_delete = sqlalchemy_delete(RelicMetadataInDB).where(RelicMetadataInDB.id == relic_id)
                await session.execute(stmt_delete)
                logger.info(f"Deleted relic metadata from DB: {relic_id}")
            
            if relic_to_delete:
                relic_directory_path = FORGED_RELICS_BASE_PATH / relic_to_delete.id
                if await aios.path.exists(relic_directory_path):
                    if await aios.path.isdir(relic_directory_path):
                        await asyncio.to_thread(shutil.rmtree, relic_directory_path)
                        logger.info(f"Deleted relic directory from filesystem: {relic_directory_path}")
                    else:
                        logger.warning(f"Relic path {relic_directory_path} is a file, not a directory. Skipping FS delete.")
                else:
                    logger.warning(f"Relic directory {relic_directory_path} not found for deletion.")
            return True

        except SQLAlchemyError as e_db:
            logger.error(f"Database error during deletion of relic {relic_id}: {e_db}", exc_info=True)
            raise 
        except Exception as e_fs:
            logger.error(f"Filesystem error deleting relic {relic_id} package/directory (DB record was already deleted): {e_fs}", exc_info=True)
            raise Exception(f"Filesystem error post-DB-commit during relic deletion: {e_fs}")
