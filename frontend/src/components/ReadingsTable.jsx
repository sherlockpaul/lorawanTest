export default function ReadingsTable({ readings }) {
  if (!readings.length) {
    return <p className="no-data">No readings yet.</p>
  }

  return (
    <div className="table-wrapper">
      <table>
        <thead>
          <tr>
            <th>Time</th>
            <th>Device</th>
            <th>Temp (°C)</th>
            <th>Humidity (%)</th>
            <th>Moisture</th>
          </tr>
        </thead>
        <tbody>
          {readings.map((r) => (
            <tr key={r.id}>
              <td>{new Date(r.received_at).toLocaleString()}</td>
              <td>{r.device_name || r.device_eui}</td>
              <td>{r.temperature?.toFixed(1) ?? '—'}</td>
              <td>{r.humidity?.toFixed(1) ?? '—'}</td>
              <td>{r.moisture ?? '—'}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}
