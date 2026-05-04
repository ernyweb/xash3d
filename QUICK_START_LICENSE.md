# License System Quick Start Guide

## Starting the Backend Server

```bash
cd backend
npm install
node server.js
```

The server will output:
```
xash3d license server running on http://0.0.0.0:3000
admin endpoints require X-Admin-Key header
```

## Generating and Testing a License

### 1. Create a Test License
```bash
ADMIN_KEY="change-me-admin-key"  # From .env

curl -X POST http://localhost:3000/api/license/issue \
  -H "X-Admin-Key: $ADMIN_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "userId": "testuser",
    "plan": "basic",
    "durationDays": 365
  }'
```

Save the returned `licenseKey` from the response.

### 2. In-Game License Setup

Start the xash3d engine/client and at the console:

```
license_set <LICENSE_KEY_FROM_ABOVE>
license_validate_online
```

If the backend is at a different location:
```
license_validate_online <LICENSE_KEY> http://your-ip:3000/api/license/validate
```

Or set the backend URL permanently:
```
license_server http://your-ip:3000
license_validate_online
```

### 3. Verify License Status
```
license_status
```

Output should show:
```
license status: online license validated
stored key: <your-key>
device id: <your-device-hash>
user: testuser plan: basic expires: <timestamp>
```

## Environment Configuration

Edit `/backend/.env`:

```env
PORT=3000
HOST=0.0.0.0
ADMIN_API_KEY=your-secure-admin-key-here
LICENSE_SERVER_SECRET=your-backend-signing-secret
TOKEN_TTL_SECONDS=86400
```

## Docker Deployment (Optional)

Create `/backend/Dockerfile`:
```dockerfile
FROM node:18-alpine
WORKDIR /app
COPY package*.json ./
RUN npm install --production
COPY server.js .
EXPOSE 3000
CMD ["node", "server.js"]
```

Build and run:
```bash
cd backend
docker build -t xash3d-license-server .
docker run -p 3000:3000 -e ADMIN_API_KEY=your-key xash3d-license-server
```

## Offline Token Format

Tokens are human-readable key=value pairs:
```
user=testuser;plan=basic;exp=1735689600;device=device-id-hash;nonce=random-nonce;sig=hex-signature
```

The signature is calculated as:
```
HMAC-SHA256(payload, secret_key).hex()

payload = "user=testuser;plan=basic;exp=1735689600;device=device-id-hash;nonce=random-nonce"
```

## Troubleshooting

### License Key Not Validating
1. Check backend is running: `curl http://localhost:3000/api/health`
2. Check server URL is correct: `license_server` cvar
3. Check device ID matches: `license_deviceid`
4. Check key format: `license_status`

### Backend Connection Failed
- Ensure firewall allows port 3000
- On macOS/Linux, check if port is in use: `lsof -i :3000`
- Verify IP/hostname resolution: `ping your-server`

### Offline Token Invalid
- Token may be expired: check `exp=timestamp` 
- Device may not match: check `device=` field
- Secret key mismatch between backend and client (check `LICENSE_SERVER_SECRET` and `s_offline_hmac_secret`)

### Database Corruption
Delete the database and restart:
```bash
rm -r backend/data
node server.js
```

## API Reference

### POST /api/license/issue (Admin)
**Headers:** `X-Admin-Key: <admin_key>`
**Body:**
```json
{
  "userId": "string",
  "plan": "string",
  "durationDays": 365,
  "deviceId": "string (optional)"
}
```
**Response (200):**
```json
{
  "licenseKey": "base64-encoded-key",
  "userId": "string",
  "plan": "string",
  "expiresAt": 1735689600,
  "offlineToken": "signed-token"
}
```

### POST /api/license/validate
**Headers:** None required
**Body:**
```json
{
  "licenseKey": "base64-encoded-key",
  "deviceId": "device-identifier"
}
```
**Response (200):**
```json
{
  "valid": true,
  "licenseKey": "string",
  "userId": "string",
  "plan": "string",
  "deviceId": "string",
  "expiresAt": 1735689600,
  "offlineToken": "signed-token"
}
```
**Response (403/404):**
```json
{
  "valid": false,
  "error": "license not found | license expired | device binding mismatch"
}
```

### POST /api/license/revoke (Admin)
**Headers:** `X-Admin-Key: <admin_key>`
**Body:**
```json
{
  "licenseKey": "base64-encoded-key"
}
```
**Response (200):**
```json
{
  "success": true,
  "licenseKey": "string"
}
```

### GET /api/license/:licenseKey (Admin)
**Headers:** `X-Admin-Key: <admin_key>`
**Response (200):**
```json
{
  "licenseKey": "string",
  "userId": "string",
  "plan": "string",
  "issuedAt": 1704067200,
  "expiresAt": 1735689600,
  "deviceId": "string or null",
  "active": true,
  "nonce": "string"
}
```

### GET /api/health
**Response (200):**
```json
{
  "status": "ok",
  "version": "1.0.0",
  "host": "0.0.0.0"
}
```
