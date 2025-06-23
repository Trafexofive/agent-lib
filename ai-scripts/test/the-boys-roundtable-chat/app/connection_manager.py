import logging
import json
from typing import List, Dict, Tuple
from fastapi import WebSocket

logger = logging.getLogger(__name__)

class ConnectionManager:
    def __init__(self):
        self.active_connections: List[Tuple[WebSocket, str]] = []

    async def connect(self, websocket: WebSocket, nickname: str):
        await websocket.accept()
        self.active_connections.append((websocket, nickname))
        logger.info(f"User '{nickname}' connected. Total connections: {len(self.active_connections)}")

    def disconnect(self, websocket: WebSocket, nickname: str):
        try:
            self.active_connections.remove((websocket, nickname))
            logger.info(f"User '{nickname}' disconnected. Total connections: {len(self.active_connections)}")
        except ValueError:
            logger.warning(f"Attempted to disconnect user '{nickname}' but connection was not found or already removed.")

    async def send_to_websocket(self, data: dict, websocket: WebSocket):
        try:
            await websocket.send_text(json.dumps(data))
        except Exception as e:
            logger.error(f"Error sending message to a websocket: {e}")
            nickname_to_remove = None
            for ws, nick in self.active_connections:
                if ws == websocket:
                    nickname_to_remove = nick
                    break
            if nickname_to_remove:
                self.disconnect(websocket, nickname_to_remove)

    async def broadcast(self, data: dict):
        message_str = json.dumps(data)
        for connection_tuple in list(self.active_connections):
            websocket_conn, nickname = connection_tuple
            try:
                await websocket_conn.send_text(message_str)
            except Exception as e:
                logger.error(f"Error broadcasting to {nickname}: {e}. Removing connection.")
                self.disconnect(websocket_conn, nickname)
