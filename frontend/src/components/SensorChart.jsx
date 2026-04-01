import {
  LineChart, Line, XAxis, YAxis, CartesianGrid,
  Tooltip, Legend, ResponsiveContainer
} from 'recharts'

function formatTime(isoString) {
  return new Date(isoString).toLocaleTimeString()
}

export default function SensorChart({ readings }) {
  if (!readings.length) return null

  const data = [...readings].reverse().map((r) => ({
    time: formatTime(r.received_at),
    temperature: r.temperature,
    humidity: r.humidity,
    moisture: r.moisture,
  }))

  return (
    <div className="charts">
      <div className="chart-block">
        <h3>Temperature (°C)</h3>
        <ResponsiveContainer width="100%" height={200}>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="time" tick={{ fontSize: 11 }} />
            <YAxis />
            <Tooltip />
            <Line type="monotone" dataKey="temperature" stroke="#e74c3c" dot={false} />
          </LineChart>
        </ResponsiveContainer>
      </div>

      <div className="chart-block">
        <h3>Humidity (%)</h3>
        <ResponsiveContainer width="100%" height={200}>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="time" tick={{ fontSize: 11 }} />
            <YAxis />
            <Tooltip />
            <Line type="monotone" dataKey="humidity" stroke="#3498db" dot={false} />
          </LineChart>
        </ResponsiveContainer>
      </div>

      <div className="chart-block">
        <h3>Moisture (raw ADC)</h3>
        <ResponsiveContainer width="100%" height={200}>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="time" tick={{ fontSize: 11 }} />
            <YAxis />
            <Tooltip />
            <Line type="monotone" dataKey="moisture" stroke="#2ecc71" dot={false} />
          </LineChart>
        </ResponsiveContainer>
      </div>
    </div>
  )
}
