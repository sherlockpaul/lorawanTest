from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from sqlalchemy import desc
from app.db.database import get_db
from app.models.sensor_data import SensorReading
from app.models.user import User
from app.api.deps import get_current_user

router = APIRouter()


@router.get("/readings")
def get_readings(
    limit: int = 100,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    """Return the most recent sensor readings across all devices."""
    readings = (
        db.query(SensorReading)
        .order_by(desc(SensorReading.received_at))
        .limit(limit)
        .all()
    )
    return readings


@router.get("/readings/{device_eui}")
def get_device_readings(
    device_eui: str,
    limit: int = 100,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    """Return the most recent readings for a specific device."""
    readings = (
        db.query(SensorReading)
        .filter(SensorReading.device_eui == device_eui)
        .order_by(desc(SensorReading.received_at))
        .limit(limit)
        .all()
    )
    return readings


@router.get("/devices")
def get_devices(
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    """Return a list of unique devices that have sent data."""
    devices = (
        db.query(SensorReading.device_eui, SensorReading.device_name)
        .distinct()
        .all()
    )
    return [{"device_eui": d.device_eui, "device_name": d.device_name} for d in devices]
