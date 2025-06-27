import asyncio
from aiohttp import web
import croniter
from datetime import datetime

async def run_scheduler(hearth):
    """Checks for scheduled workflows every minute."""
    print("[SCHEDULER] Starting cron-based scheduler.")
    while True:
        await asyncio.sleep(1) # Initial sleep to prevent race condition on startup
        now = datetime.now()
        # Align to the start of the next minute
        wait_seconds = 60 - now.second
        await asyncio.sleep(wait_seconds)
        
        schedule_now = datetime.now()
        for name, workflow in hearth.workflows.items():
            for trigger in workflow.get('triggers', []):
                if 'schedule' in trigger:
                    cron = trigger['schedule']
                    if croniter.croniter.is_valid(cron) and croniter.croniter(cron, schedule_now).match(schedule_now):
                         print(f"[SCHEDULER] Triggering workflow '{name}' on schedule '{cron}'.")
                         trigger_data = {"type": "schedule", "timestamp": schedule_now.isoformat()}
                         hearth.schedule_workflow_run(name, trigger_data)


async def run_webhook_server(hearth):
    """Runs a web server to listen for webhook triggers."""
    app = web.Application()

    async def handle_webhook(request):
        workflow_name = request.match_info.get('workflow_name')
        if not workflow_name:
            return web.Response(text="Workflow name must be specified in the URL.", status=400)
        
        if workflow_name not in hearth.workflows:
            return web.Response(text=f"Workflow '{workflow_name}' not found.", status=404)
        
        try:
            inputs = await request.json() if request.can_read_body else {}
        except Exception:
            inputs = {}
            
        print(f"[WEBHOOK] Triggering workflow '{workflow_name}' from webhook.")
        trigger_data = {
            "type": "webhook",
            "path": request.path,
            "headers": dict(request.headers),
            "inputs": inputs
        }
        hearth.schedule_workflow_run(workflow_name, trigger_data, inputs)
        return web.Response(text="Workflow triggered.", status=202)

    app.add_routes([web.post('/workflows/run/{workflow_name}', handle_webhook)])
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, '0.0.0.0', 8000)
    print("[WEBHOOK] Starting webhook server on http://0.0.0.0:8000")
    await site.start()
