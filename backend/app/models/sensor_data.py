from sqlalchemy import Column, String, Float, Integer, DateTime
from sqlalchemy.sql import func
from app.db.database import Base


class SensorReading(Base):
    __tablename__ = "sensor_readings"

    id          = Column(Integer, primary_key=True, index=True)
    device_eui  = Column(String, index=True, nullable=False)
    device_name = Column(String, nullable=True)
    temperature = Column(Float, nullable=True)   # Celsius
    humidity    = Column(Float, nullable=True)   # Percent
    moisture    = Column(Integer, nullable=True) # Raw ADC value
    received_at = Column(DateTime(timezone=True), server_default=func.now())
