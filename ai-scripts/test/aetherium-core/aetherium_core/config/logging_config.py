import os, logging
from logging.config import dictConfig
from python_json_logger import jsonlogger

def setup_logging():
    # This version imports jsonlogger and uses the class object directly,
    # which fixes the 'Cannot resolve' ValueError.
    dictConfig({
        'version': 1,
        'disable_existing_loggers': False,
        'formatters': {'json': {'()': jsonlogger.JsonFormatter, 'format': '%(asctime)s %(levelname)s %(name)s %(message)s'}},
        'handlers': {
            'json': {'class': 'logging.handlers.RotatingFileHandler', 'formatter': 'json', 'filename': '/app/logs/aetherium_core.log', 'maxBytes': 10485760, 'backupCount': 5},
            'console': {'class': 'logging.StreamHandler', 'formatter': 'json'},
        },
        'loggers': {'': {'handlers': ['json', 'console'], 'level': os.getenv('LOG_LEVEL', 'INFO').upper()}}
    })
