import logging
import datetime
import json
from typing import List, Dict, Any
from contextlib import asynccontextmanager

from fastapi import FastAPI, WebSocket, WebSocketDisconnect, Request
from fastapi.responses import FileResponse, HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles

from app.config import settings
from app.connection_manager import ConnectionManager

logging.basicConfig(level=settings.LOG_LEVEL.upper())
logger = logging.getLogger(__name__)

manager = ConnectionManager()
chat_history: List[Dict[str, Any]] = []

@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info(f"Starting '{settings.CHAT_TITLE}' on port {settings.APP_PORT}")
    logger.info(f"Max chat history: {settings.MAX_HISTORY}")
    yield
    logger.info(f"Shutting down '{settings.CHAT_TITLE}'")

app = FastAPI(lifespan=lifespan, title=settings.CHAT_TITLE)

app.mount("/static", StaticFiles(directory="app/static"), name="static")

@app.get("/", response_class=HTMLResponse)
async def get_chat_ui(request: Request):
    return FileResponse("app/static/index.html")

@app.get("/app-config")
async def get_app_config():
    return {
        "chat_title": settings.CHAT_TITLE,
        "max_history": settings.MAX_HISTORY
    }

@app.websocket("/ws/{nickname}")
async def websocket_endpoint(websocket: WebSocket, nickname: str):
    await manager.connect(websocket, nickname)
    
    safe_nickname = nickname.replace('<', '<').replace('>', '>')[:30]
    
    join_text = safe_nickname + " just landed in the Roundtable. Watch your six."
    join_message = {
        "type": "system_notification",
        "nickname": "System",
        "text": join_text,
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "flair": "join"
    }
    await manager.broadcast(join_message)
    chat_history.append(join_message)
    if len(chat_history) > settings.MAX_HISTORY:
        chat_history.pop(0)

    if chat_history:
        history_payload = {"type": "history_batch", "messages": chat_history[-settings.MAX_HISTORY:]}
        await manager.send_to_websocket(history_payload, websocket)

    try:
        while True:
            data = await websocket.receive_text()
            try:
                message_data = json.loads(data)
                text_content = message_data.get("text", "")
            except json.JSONDecodeError:
                text_content = data 

            if not text_content.strip():
                continue

            message = {
                "type": "message",
                "nickname": safe_nickname, 
                "text": text_content.replace('<', '<').replace('>', '>')[:500],
                "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "flair": "user"
            }
            await manager.broadcast(message)
            chat_history.append(message)
            if len(chat_history) > settings.MAX_HISTORY:
                chat_history.pop(0)

    except WebSocketDisconnect:
        manager.disconnect(websocket, safe_nickname)
        leave_text = safe_nickname + " has gone dark. Probably for the best."
        leave_message = {
            "type": "system_notification",
            "nickname": "System",
            "text": leave_text,
            "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "flair": "leave"
        }
        await manager.broadcast(leave_message)
        chat_history.append(leave_message)
        if len(chat_history) > settings.MAX_HISTORY:
            chat_history.pop(0)
    except Exception as e:
        logger.error(f"Error with WebSocket connection for {safe_nickname}: {e}", exc_info=True)
        manager.disconnect(websocket, safe_nickname)

@app.get("/health")
async def health_check():
    return JSONResponse({"status": "Vought-fully Operational", "chat_title": settings.CHAT_TITLE})

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("app.main:app", host="0.0.0.0", port=settings.APP_PORT, reload=True, log_level=settings.LOG_LEVEL.lower())
