export default function DeviceList({ devices, selected, onSelect }) {
  if (!devices.length) {
    return <p className="no-data">No devices have sent data yet.</p>
  }

  return (
    <div className="device-list">
      <button
        className={`device-card ${selected === null ? 'active' : ''}`}
        onClick={() => onSelect(null)}
      >
        All Devices
      </button>
      {devices.map((d) => (
        <button
          key={d.device_eui}
          className={`device-card ${selected === d.device_eui ? 'active' : ''}`}
          onClick={() => onSelect(d.device_eui)}
        >
          <span className="device-name">{d.device_name || d.device_eui}</span>
          <span className="device-eui">{d.device_eui}</span>
        </button>
      ))}
    </div>
  )
}
