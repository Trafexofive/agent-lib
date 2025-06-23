# GOLD ESSENCE: Absolute Sovereignty - Direct control over the host machine.
import subprocess, logging
logger = logging.getLogger(__name__)

class SystemCommandTool(BaseTool):
    @property
    def name(self) -> str: return "system.execute"

    async def execute(self, params: dict):
        command = params.get('command')
        if not command:
            raise ValueError("'command' parameter is required.")
        try:
            result = subprocess.run(command, shell=True, capture_output=True, text=True, check=True, timeout=60)
            return {'stdout': result.stdout, 'stderr': result.stderr, 'return_code': result.returncode}
        except subprocess.CalledProcessError as e:
            logger.error(f"Command failed with exit code {e.returncode}: {e.stderr}")
            return {'stdout': e.stdout, 'stderr': e.stderr, 'return_code': e.returncode}
