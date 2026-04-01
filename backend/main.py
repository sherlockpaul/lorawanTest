from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from contextlib import asynccontextmanager

from app.db.database import init_db
from app.mqtt.client import start_mqtt, stop_mqtt
from app.api.routes import router
from app.api.auth import router as auth_router


@asynccontextmanager
async def lifespan(app: FastAPI):
    init_db()
    start_mqtt()
    yield
    stop_mqtt()


app = FastAPI(title="LoRaWAN Dashboard API", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173"],  # Vite dev server
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(auth_router, prefix="/api")
app.include_router(router, prefix="/api")
