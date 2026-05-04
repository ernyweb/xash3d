# Xash3D License System Integration

## Overview
This document describes the complete backend-based license system integration for the xash3d game engine. The system enforces online license validation at startup and blocks gameplay until a valid license is verified.

## Architecture

### Backend Server (Node.js/Express)
**Location:** `/backend/server.js`

**Endpoints:**
- `POST /api/license/issue` — Issue a new license key (admin-protected)
- `POST /api/license/validate` — Validate a license key and issue an offline token
- `GET /api/license/<licenseKey>` — Fetch license details (admin-protected)
- `POST /api/license/revoke` — Revoke a license key (admin-protected)
- `GET /api/health` — Health check

**Default Configuration:**
- Host: `0.0.0.0`
- Port: `3000`
- Admin key header: `X-Admin-Key` (set via `.env`)
- Database: File-based JSON (`data/licenses.json`)

### Client License Module
**Location:** `/engine/client/license.c` and `/engine/client/license.h`

**Features:**
- Online license validation via HTTP POST to backend
- Offline token generation and signature verification (HMAC-SHA256)
- Automatic offline token caching
- Device binding (optional per license)
- Configurable server URL via cvar

**Key Functions:**
- `License_ValidateOnline()` — HTTP POST validation against backend
- `License_ValidateOfflineToken()` — Verify cached offline token
- `License_Enforce()` — Force console mode if license invalid
- `License_Draw()` — Display license information overlay

## Usage Flow

### 1. License Issuance (Backend Admin)
```bash
curl -X POST http://localhost:3000/api/license/issue \
  -H "X-Admin-Key: change-me-admin-key" \
  -H "Content-Type: application/json" \
  -d '{
    "userId": "user123",
    "plan": "basic",
    "durationDays": 365,
    "deviceId": "optional-device-binding"
  }'
```

**Response:**
```json
{
  "licenseKey": "a1b2c3d4e5f6...",
  "offlineToken": "user=user123;plan=basic;exp=1735689600;device=optional-device-binding;nonce=xxx;sig=yyy",
  "expiresAt": 1735689600
}
```

### 2. Client License Setup (In-Game Console)
```
license_set a1b2c3d4e5f6...
license_validate_online
```

Or with custom server:
```
license_validate_online a1b2c3d4e5f6... http://your-server:3000/api/license/validate
```

### 3. License Verification
The client:
1. Reads the stored license key
2. Sends HTTP POST to `/api/license/validate` with `licenseKey` and `deviceId`
3. Backend verifies the key against the database
4. Server returns signed offline token
5. Client validates the token signature with `HMAC-SHA256`
6. Token is cached locally and gameplay is unblocked

### 4. Offline Operation
If the backend is unreachable:
- Client uses the previously cached offline token
- Token signature is validated locally
- If token is valid and not expired, gameplay proceeds
- If no cached token or token is invalid/expired, license screen persists

## Client-Side Integration

### Console Commands
- `license_set <key>` — Store a license key
- `license_clear` — Clear the stored key
- `license_status` — Show current license status
- `license_validate_online [key] [server_url]` — Validate against backend
- `license_validate_offline <token>` — Validate a signed offline token
- `license_deviceid` — Show the device identifier
- `license_help` — Show command reference

### CVars
- `license_server` — Backend server URL (default: `http://72.60.130.39:3000`)

### Rendering Integration
The license module is integrated into the main rendering pipeline:
- **File:** `/engine/client/cl_view.c`
- **Function:** `V_PostRender()`
- Calls to `License_Enforce()` and `License_Draw()` during every frame

### Initialization
License system is initialized during console initialization:
- **File:** `/engine/client/console.c`
- **Function:** `Con_Init()`
- Calls `License_Init()` to register commands and load stored keys/tokens

## Security Considerations

### Endpoint Protection
- License issue/revoke/detail endpoints require `X-Admin-Key` header
- Rate limiting: 40 requests per 60 seconds (configurable)
- Request body limit: 32KB

### Token Signing
- Offline tokens are signed with `HMAC-SHA256`
- Secret key: `xash3d-offline-token-demo-secret-change-me` (change in production)
- Token includes:
  - `user_id`, `plan`, `expires`, `device_id`, `nonce`
  - `sig` — hex-encoded HMAC-SHA256 signature

### Device Binding
- Optional per-license device ID
- If not set on first validation, binds to the validating device
- Subsequent validations must come from the same device (prevents key sharing)

## Deployment

### Backend Setup
```bash
cd backend
npm install
cp .env.example .env
# Edit .env and set:
# - ADMIN_API_KEY (change the default immediately)
# - LICENSE_SERVER_SECRET (for token signing)
# - PORT and HOST
node server.js
```

### Client Configuration
Set up VPS deployment:
1. Deploy backend to `http://your-vps-ip:3000`
2. In client, set: `license_server http://your-vps-ip:3000`
3. Users run: `license_set <key>` then `license_validate_online`

## Network Traffic

### License Validation Request (HTTP POST)
```
POST /api/license/validate HTTP/1.1
Host: license-server:3000
Content-Type: application/json
Content-Length: 128

{"licenseKey":"a1b2c3d4e5f6...","deviceId":"device-md5-hash"}
```

### License Validation Response (on success)
```
HTTP/1.1 200 OK
Content-Type: application/json

{
  "valid": true,
  "offlineToken": "user=...;sig=...",
  "expiresAt": 1735689600
}
```

## Testing

### Backend Health Check
```bash
curl http://localhost:3000/api/health
# {"status":"ok","version":"1.0.0","host":"0.0.0.0"}
```

### Issue a Test License
```bash
curl -X POST http://localhost:3000/api/license/issue \
  -H "X-Admin-Key: your-admin-key" \
  -H "Content-Type: application/json" \
  -d '{"userId":"test_user","plan":"basic","durationDays":30}'
```

### Validate a License
```bash
curl -X POST http://localhost:3000/api/license/validate \
  -H "Content-Type: application/json" \
  -d '{"licenseKey":"key_from_above","deviceId":"test-device"}'
```

## File Changes Summary

| File | Changes |
|------|---------|
| `engine/client/license.c` | Added `License_ValidateOnline()`, HTTP POST implementation, JSON parsing, `License_Draw()`, `License_Enforce()`, offline token persistence |
| `engine/client/license.h` | Added new function declarations for `License_IsValid()`, `License_Draw()`, `License_Enforce()` |
| `engine/client/cl_view.c` | Added `#include "license.h"`, calls to `License_Enforce()` and `License_Draw()` in rendering pipeline |
| `backend/server.js` | Complete Node.js/Express license backend with all endpoints |

## Status
- ✅ Backend server fully implemented
- ✅ Client online validation implemented
- ✅ Offline token caching implemented
- ✅ License blocking enforcement integrated
- ✅ HTTP POST from client to backend working
- ✅ Device binding supported
- ✅ Device ID generation using engine's ID_GetMD5()

## Notes
- The secret used for HMAC signing is hardcoded in client code. In production, ensure the backend and client use the same secret.
- The HTTP implementation does NOT support HTTPS in this version (only HTTP).
- Rate limiting is applied globally to the backend (not per-API-key).
- Offline token TTL is configurable via `TOKEN_TTL_SECONDS` environment variable.
