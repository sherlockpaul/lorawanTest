import { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { getDevices, getReadings } from '../api'
import DeviceList from '../components/DeviceList'
import ReadingsTable from '../components/ReadingsTable'
import SensorChart from '../components/SensorChart'

export default function Dashboard() {
  const { user, logout } = useAuth()
  const [devices, setDevices] = useState([])
  const [readings, setReadings] = useState([])
  const [selectedDevice, setSelectedDevice] = useState(null)
  const [lastUpdated, setLastUpdated] = useState(null)

  async function fetchData(deviceEui) {
    const [devs, reads] = await Promise.all([
      getDevices(),
      getReadings(deviceEui),
    ])
    setDevices(devs)
    setReadings(reads)
    setLastUpdated(new Date().toLocaleTimeString())
  }

  useEffect(() => {
    fetchData(selectedDevice)
    const interval = setInterval(() => fetchData(selectedDevice), 15000)
    return () => clearInterval(interval)
  }, [selectedDevice])

  return (
    <div className="app">
      <header>
        <h1>LoRaWAN Dashboard</h1>
        <div className="header-right">
          {lastUpdated && <span className="last-updated">Updated: {lastUpdated}</span>}
          <span className="user-email">{user?.email}</span>
          <button className="logout-btn" onClick={logout}>Sign out</button>
        </div>
      </header>

      <section className="section">
        <h2>Devices</h2>
        <DeviceList
          devices={devices}
          selected={selectedDevice}
          onSelect={setSelectedDevice}
        />
      </section>

      <section className="section">
        <h2>Charts</h2>
        <SensorChart readings={readings} />
      </section>

      <section className="section">
        <h2>Recent Readings</h2>
        <ReadingsTable readings={readings} />
      </section>
    </div>
  )
}
