const express = require('express');
const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
require('dotenv').config();

const PORT = parseInt(process.env.PORT || '3000', 10);
const HOST = process.env.HOST || '0.0.0.0';
const ADMIN_API_KEY = process.env.ADMIN_API_KEY || 'change-me-admin-key';
const SECRET = process.env.LICENSE_SERVER_SECRET || 'xash3d-offline-token-demo-secret-change-me';
const DB_FILE = path.join(__dirname, 'data', 'licenses.json');
const TOKEN_TTL_SECONDS = parseInt(process.env.TOKEN_TTL_SECONDS || '86400', 10);

const app = express();
app.disable('x-powered-by');
app.use(helmet());
app.use(express.json({ limit: '32kb' }));
app.use(express.urlencoded({ extended: false }));

const limiter = rateLimit({
  windowMs: 60 * 1000,
  max: 40,
  standardHeaders: true,
  legacyHeaders: false,
});
app.use(limiter);

function ensureDatabase() {
  const dir = path.dirname(DB_FILE);
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
  if (!fs.existsSync(DB_FILE)) {
    fs.writeFileSync(DB_FILE, JSON.stringify({ licenses: [] }, null, 2), 'utf8');
  }
}

function loadDatabase() {
  ensureDatabase();
  const data = fs.readFileSync(DB_FILE, 'utf8');
  return JSON.parse(data || '{"licenses":[]}');
}

function saveDatabase(db) {
  fs.writeFileSync(DB_FILE, JSON.stringify(db, null, 2), 'utf8');
}

function generateNonce() {
  return crypto.randomBytes(16).toString('hex');
}

function generateLicenseKey() {
  return crypto.randomBytes(20).toString('hex');
}

function signPayload(payload) {
  return crypto.createHmac('sha256', SECRET).update(payload, 'utf8').digest('hex');
}

function buildOfflineToken({ userId, plan, expires, deviceId, nonce }) {
  const payload = `user=${userId};plan=${plan};exp=${expires};device=${deviceId};nonce=${nonce}`;
  const signature = signPayload(payload);
  return `${payload};sig=${signature}`;
}

function requireAdminKey(req, res, next) {
  const apiKey = req.header('x-admin-key');
  if (!apiKey || apiKey !== ADMIN_API_KEY) {
    return res.status(401).json({ error: 'Unauthorized: invalid admin key' });
  }
  next();
}

app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', version: '1.0.0', host: HOST });
});

app.post('/api/license/issue', requireAdminKey, (req, res) => {
  const { userId, plan, durationDays = 365, deviceId = '' } = req.body;
  if (!userId || !plan) {
    return res.status(400).json({ error: 'userId and plan are required' });
  }

  const now = Math.floor(Date.now() / 1000);
  const expires = now + Math.max(1, parseInt(durationDays, 10)) * 24 * 3600;
  const licenseKey = generateLicenseKey();
  const nonce = generateNonce();
  const token = buildOfflineToken({ userId, plan, expires, deviceId: deviceId || 'unbound', nonce });

  const db = loadDatabase();
  db.licenses.push({
    licenseKey,
    userId,
    plan,
    issuedAt: now,
    expiresAt: expires,
    deviceId: deviceId || null,
    active: true,
    nonce,
  });
  saveDatabase(db);

  res.json({
    licenseKey,
    userId,
    plan,
    expiresAt: expires,
    offlineToken: token,
  });
});

app.post('/api/license/validate', (req, res) => {
  const { licenseKey, deviceId } = req.body;
  if (!licenseKey || !deviceId) {
    return res.status(400).json({ error: 'licenseKey and deviceId are required' });
  }

  const now = Math.floor(Date.now() / 1000);
  const db = loadDatabase();
  const entry = db.licenses.find((item) => item.licenseKey === licenseKey && item.active);

  if (!entry) {
    return res.status(404).json({ valid: false, error: 'license not found' });
  }

  if (entry.expiresAt < now) {
    return res.status(403).json({ valid: false, error: 'license expired' });
  }

  if (entry.deviceId && entry.deviceId !== deviceId) {
    return res.status(403).json({ valid: false, error: 'device binding mismatch' });
  }

  if (!entry.deviceId) {
    entry.deviceId = deviceId;
    saveDatabase(db);
  }

  const tokenExpires = now + TOKEN_TTL_SECONDS;
  const nonce = generateNonce();
  const offlineToken = buildOfflineToken({
    userId: entry.userId,
    plan: entry.plan,
    expires: tokenExpires,
    deviceId,
    nonce,
  });

  res.json({
    valid: true,
    licenseKey: entry.licenseKey,
    userId: entry.userId,
    plan: entry.plan,
    deviceId,
    expiresAt: entry.expiresAt,
    offlineToken,
  });
});

app.post('/api/license/revoke', requireAdminKey, (req, res) => {
  const { licenseKey } = req.body;
  if (!licenseKey) {
    return res.status(400).json({ error: 'licenseKey is required' });
  }

  const db = loadDatabase();
  const entry = db.licenses.find((item) => item.licenseKey === licenseKey);
  if (!entry) {
    return res.status(404).json({ error: 'license not found' });
  }

  entry.active = false;
  saveDatabase(db);
  res.json({ success: true, licenseKey });
});

app.get('/api/license/:licenseKey', requireAdminKey, (req, res) => {
  const { licenseKey } = req.params;
  const db = loadDatabase();
  const entry = db.licenses.find((item) => item.licenseKey === licenseKey);
  if (!entry) {
    return res.status(404).json({ error: 'license not found' });
  }
  res.json(entry);
});

app.use((req, res) => {
  res.status(404).json({ error: 'endpoint not found' });
});

app.listen(PORT, HOST, () => {
  console.log(`xash3d license server running on http://${HOST}:${PORT}`);
  console.log(`admin endpoints require X-Admin-Key header`);
});
