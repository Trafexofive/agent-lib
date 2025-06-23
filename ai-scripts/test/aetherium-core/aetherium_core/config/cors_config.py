import os
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

def setup_cors(app: FastAPI):
    origins = os.getenv("CORS_ALLOWED_ORIGINS", "").split()
    app.add_middleware(CORSMiddleware, allow_origins=origins if origins else ["*"], allow_credentials=True, allow_methods=["*"], allow_headers=["*"],)
