import logging
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base

from app.config_platform import settings

logger = logging.getLogger(__name__)

Base = declarative_base()
_ASYNC_ENGINE = None
_AsyncSessionFactory = None

def get_engine():
    global _ASYNC_ENGINE
    if _ASYNC_ENGINE is None:
        logger.info(f"Creating SQLAlchemy engine for: {settings.PLATFORM_SQLALCHEMY_DATABASE_URL}")
        _ASYNC_ENGINE = create_async_engine(
            settings.PLATFORM_SQLALCHEMY_DATABASE_URL,
            echo=(settings.LOG_LEVEL.upper() == "DEBUG")
        )
    return _ASYNC_ENGINE

def get_async_session_factory():
    global _AsyncSessionFactory
    if _AsyncSessionFactory is None:
        engine = get_engine()
        _AsyncSessionFactory = sessionmaker(engine, class_=AsyncSession, expire_on_commit=False)
    return _AsyncSessionFactory

async def get_db():
    factory = get_async_session_factory()
    async with factory() as session:
        try:
            yield session
        finally:
            await session.close()

async def init_db():
    engine = get_engine()
    async with engine.begin() as conn:
        from app.models_platform import AgentDB, ReportDB
        await conn.run_sync(Base.metadata.create_all)
    logger.info("Database tables checked/created.")

async def close_db_connection():
    global _ASYNC_ENGINE
    if _ASYNC_ENGINE:
        await _ASYNC_ENGINE.dispose()
        _ASYNC_ENGINE = None
        logger.info("SQLAlchemy engine disposed.")
