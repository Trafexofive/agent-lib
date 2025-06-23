import datetime
from app.config_forge import settings # Updated import name

class SystemInfo:
    def get_current_timestamp(self) -> str:
        return datetime.datetime.now(datetime.timezone.utc).isoformat()

    def get_health_status(self) -> dict:
        return {
            "service_name": settings.app_name,
            "version": settings.app_version,
            "status": "healthy", 
            "timestamp": self.get_current_timestamp(),
            "environment": settings.app_env
        }

    def get_capabilities(self) -> dict:
        # This should ideally list actual implemented API endpoints
        return {
            "service_name": settings.app_name,
            "version": settings.app_version,
            "api_description": "Provides API for forging software relics from plans.",
            "endpoints": [
                {"method": "POST", "path": "/forge/relic", "description": "Forge a new relic from a plan (JSON body: RelicPlan)", "response": "RelicMetadata"},
                {"method": "GET", "path": "/forge/relics", "description": "List all forged relics", "response": "List[RelicMetadata]"},
                {"method": "GET", "path": "/forge/relics/{relic_id}/download", "description": "Download a forged relic package (.tar.gz)"},
                {"method": "DELETE", "path": "/forge/relics/{relic_id}", "description": "Delete a forged relic"},
                {"method": "GET", "path": "/system/health", "description": "Check system health status"},
                {"method": "GET", "path": "/system/capabilities", "description": "List system capabilities and API info"}
            ],
            "yt_dlp_integration_status": "yt-dlp is a dependency, but not actively used by the current API endpoints."
        }
