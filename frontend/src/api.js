const BASE = 'http://192.168.0.149:8000/api'

function authHeaders() {
  const token = localStorage.getItem('token')
  return token ? { Authorization: `Bearer ${token}` } : {}
}

async function handleResponse(res) {
  if (!res.ok) {
    const body = await res.json().catch(() => ({}))
    throw new Error(body.detail || `Request failed (${res.status})`)
  }
  return res.json()
}

export async function registerUser(email, password) {
  const res = await fetch(`${BASE}/auth/register`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ email, password }),
  })
  const data = await handleResponse(res)
  return { token: data.access_token, user: { email } }
}

export async function loginUser(email, password) {
  const res = await fetch(`${BASE}/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ email, password }),
  })
  const data = await handleResponse(res)
  return { token: data.access_token, user: { email } }
}

export async function getDevices() {
  const res = await fetch(`${BASE}/devices`, { headers: authHeaders() })
  return handleResponse(res)
}

export async function getReadings(deviceEui = null, limit = 50) {
  const url = deviceEui
    ? `${BASE}/readings/${deviceEui}?limit=${limit}`
    : `${BASE}/readings?limit=${limit}`
  const res = await fetch(url, { headers: authHeaders() })
  return handleResponse(res)
}
