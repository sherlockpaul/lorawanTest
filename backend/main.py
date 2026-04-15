from pathlib import Path
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from contextlib import asynccontextmanager

from app.db.database import init_db
from app.mqtt.client import start_mqtt, stop_mqtt
from app.api.routes import router
from app.api.auth import router as auth_router

STATIC_DIR = Path(__file__).parent / "static"


@asynccontextmanager
async def lifespan(app: FastAPI):
    init_db()
    start_mqtt()
    yield
    stop_mqtt()


app = FastAPI(title="LoRaWAN Dashboard API", lifespan=lifespan)

# Allow Vite dev server during local development
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173"],
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(auth_router, prefix="/api")
app.include_router(router, prefix="/api")

# Serve the built React app — must come after API routes
if STATIC_DIR.exists():
    app.mount("/", StaticFiles(directory=str(STATIC_DIR), html=True), name="frontend")
