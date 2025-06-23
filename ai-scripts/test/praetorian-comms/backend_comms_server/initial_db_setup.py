import logging
from database import engine, Base

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def init_db():
    logger.info("Creating database tables if they do not exist...")
    try:
        # The following import is crucial as it registers the models with SQLAlchemy's Base
        import db_models
        Base.metadata.create_all(bind=engine)
        logger.info("Database tables checked/created successfully.")
    except Exception as e:
        logger.error("An error occurred while creating database tables.")
        logger.error(e, exc_info=True)
        # In a real-world scenario, you might want to exit if the DB setup fails.
        # For this relic, we'll log the error and allow the app to attempt to start.

if __name__ == "__main__":
    init_db()
