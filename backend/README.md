.zip# xash3d License Server

A minimal Node.js backend for issuing and validating xash3d license keys and offline tokens.

## Purpose
- `POST /api/license/issue` issues a new license and returns a signed offline token.
- `POST /api/license/validate` validates a stored license key and device ID, then returns a new offline token.
- `POST /api/license/revoke` disables a license.
- `GET /api/license/:licenseKey` reads license metadata.

## Install
On your Ubuntu VPS:

```bash
cd /workspaces/xash3d/backend
npm install
cp .env.example .env
# edit .env and set ADMIN_API_KEY and LICENSE_SERVER_SECRET
npm start
```

## Recommended production settings
- Use a strong `ADMIN_API_KEY`
- Use a long, unique `LICENSE_SERVER_SECRET`
- Run server on `0.0.0.0` and forward port `3000` from your firewall
- Use a process manager such as `pm2` or `systemd`

## Run with screen
A helper script is available to start the server in a detached `screen` session:

```bash
cd /workspaces/xash3d/backend
./start_license_server.sh
```

Then attach to the session with:

```bash
screen -r xash-license-server
```

## Example request

### Issue a license

```bash
curl -X POST http://72.60.130.39:3000/api/license/issue \
  -H "Content-Type: application/json" \
  -H "X-Admin-Key: your-admin-key" \
  -d '{"userId":"player123","plan":"pro","durationDays":30,"deviceId":"DEVICE_ID_HERE"}'
```

### Validate a license

```bash
curl -X POST http://72.60.130.39:3000/api/license/validate \
  -H "Content-Type: application/json" \
  -d '{"licenseKey":"...","deviceId":"DEVICE_ID_HERE"}'
```

## Notes
- This server uses the same offline token format expected by the current client implementation in `engine/client/license.c`.
- For production, set `LICENSE_SERVER_SECRET` to the same value the client uses for HMAC verification.
- The client currently verifies offline tokens using a shared secret; for stronger security you should later migrate to asymmetric signing.
