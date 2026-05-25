'use strict';

// =============================================================================
// OpenNerfESC - app.js
// Web Bluetooth client + i18n (en/pl).
// =============================================================================

// --- i18n ---
const STRINGS = {
  en: {
    title:        'OpenNerfESC Configurator',
    disconnected: 'Disconnected',
    label_spin:   'Spin-up time',
    label_speed:  'Target speed',
    label_volt:   'Min. voltage',
    btn_connect:  'Connect to ESC',
    btn_connect_active: 'Disconnect',
    btn_send:     'Send parameters',
    github_link:  'GitHub project',
    log_searching:   'Searching…',
    log_connecting:  'Connecting…',
    log_connected:   'Connected to ',
    log_disconnected:'Device disconnected',
    log_read:     'Read from device: spinUp={0}ms speed={1}% volt={2}V',
    log_read_err: 'Could not read values: ',
    log_sent:     'Sent: spinUp={0}ms speed={1}% volt={2}V',
    log_write_err:'Write error: ',
    log_conn_err: 'Connection error: ',
    err_no_bt:    'This browser does not support Web Bluetooth. Use Chrome/Edge on desktop.',
    err_spin:     'Spin-up time out of range (0-500 ms)',
    err_speed:    'Target speed out of range (0-100 %)',
    err_volt:     'Min. voltage out of range (3.0-15.0 V)',
    status_connecting: 'Connecting…',
    status_searching:  'Searching…',
    status_conn_err:   'Connection error',
    status_write_err:  'Write error',
  },
  pl: {
    title:        'OpenNerfESC Konfigurator',
    disconnected: 'Rozłączono',
    label_spin:   'Czas rozpędu',
    label_speed:  'Prędkość docelowa',
    label_volt:   'Min. napięcie',
    btn_connect:  'Połącz z ESC',
    btn_connect_active: 'Rozłącz',
    btn_send:     'Wyślij parametry',
    github_link:  'Projekt na GitHub',
    log_searching:   'Szukam urządzenia…',
    log_connecting:  'Łączę…',
    log_connected:   'Połączono z ',
    log_disconnected:'Urządzenie rozłączone',
    log_read:     'Odczytano: spinUp={0}ms speed={1}% volt={2}V',
    log_read_err: 'Nie udało się odczytać: ',
    log_sent:     'Wysłano: spinUp={0}ms speed={1}% volt={2}V',
    log_write_err:'Błąd zapisu: ',
    log_conn_err: 'Błąd połączenia: ',
    err_no_bt:    'Ta przeglądarka nie obsługuje Web Bluetooth. Użyj Chrome/Edge na desktopie.',
    err_spin:     'Czas rozpędu poza zakresem (0-500 ms)',
    err_speed:    'Prędkość poza zakresem (0-100 %)',
    err_volt:     'Napięcie poza zakresem (3.0-15.0 V)',
    status_connecting: 'Łączę…',
    status_searching:  'Szukam…',
    status_conn_err:   'Błąd połączenia',
    status_write_err:  'Błąd zapisu',
  }
};

// Detect browser language (pl if starts with 'pl', otherwise en)
let lang = (navigator.language || 'en').startsWith('pl') ? 'pl' : 'en';

function t(key, ...args) {
  let s = (STRINGS[lang][key] || STRINGS.en[key] || key);
  args.forEach((v, i) => { s = s.replace('{' + i + '}', v); });
  return s;
}

function applyLang() {
  document.title = t('title');
  document.documentElement.lang = lang;
  document.querySelectorAll('[data-i18n]').forEach(el => {
    const k = el.dataset.i18n;
    if (k === 'disconnected' && bleDevice && bleDevice.gatt.connected) return;
    el.textContent = t(k);
  });
  document.getElementById('btn-lang').textContent = lang === 'en' ? 'PL' : 'EN';
  const ghLink = document.querySelector('.gh-link');
  if (ghLink) ghLink.title = t('github_link');
  // Update connect button text based on state
  const bc = document.getElementById('btn-connect');
  if (bleDevice && bleDevice.gatt.connected) {
    bc.textContent = t('btn_connect_active');
  } else {
    bc.textContent = t('btn_connect');
  }
}

document.getElementById('btn-lang').addEventListener('click', () => {
  lang = lang === 'en' ? 'pl' : 'en';
  applyLang();
});

// --- BLE UUIDs (must match firmware/include/config.h) ---
const SERVICE_UUID      = '12345678-1234-1234-1234-123456789abc';
const CHAR_SPIN_UP_TIME = '12345678-1234-1234-1234-123456789001';
const CHAR_TARGET_SPEED = '12345678-1234-1234-1234-123456789002';
const CHAR_MIN_VOLTAGE  = '12345678-1234-1234-1234-123456789003';

// --- DOM ---
const btnConnect   = document.getElementById('btn-connect');
const btnSend      = document.getElementById('btn-send');
const statusBadge  = document.getElementById('status');
const inputSpin    = document.getElementById('input-spin-up-time');
const inputSpeed   = document.getElementById('input-target-speed');
const inputVoltage = document.getElementById('input-min-voltage');
const spinValue    = document.getElementById('spin-value');
const speedValue   = document.getElementById('speed-value');
const voltageValue = document.getElementById('voltage-value');
const logEl        = document.getElementById('log');

// --- State ---
let bleDevice = null;
let chars = {};

// --- Helpers ---
function setStatus(key, cls) {
  statusBadge.textContent = t(key);
  statusBadge.className = 'status ' + (cls || '');
}
function setStatusText(text, cls) {
  statusBadge.textContent = text;
  statusBadge.className = 'status ' + (cls || '');
}
function addLog(msg) {
  const d = document.createElement('div');
  d.textContent = new Date().toLocaleTimeString() + '  ' + msg;
  logEl.prepend(d);
  if (logEl.children.length > 50) logEl.lastChild.remove();
}
function uint16LE(v) { const b = new ArrayBuffer(2); new DataView(b).setUint16(0, v, true); return b; }
function uint8Buf(v) { const b = new ArrayBuffer(1); new DataView(b).setUint8(0, v); return b; }
function float32LE(v) { const b = new ArrayBuffer(4); new DataView(b).setFloat32(0, v, true); return b; }

// --- Slider live display ---
inputSpin.addEventListener('input',    () => spinValue.textContent    = inputSpin.value + ' ms');
inputSpeed.addEventListener('input',   () => speedValue.textContent   = inputSpeed.value + '%');
inputVoltage.addEventListener('input', () => voltageValue.textContent = parseFloat(inputVoltage.value).toFixed(1) + ' V');

// --- Connect / Disconnect ---
btnConnect.addEventListener('click', async () => {
  if (!navigator.bluetooth) { alert(t('err_no_bt')); return; }
  if (bleDevice && bleDevice.gatt.connected) { bleDevice.gatt.disconnect(); return; }
  try {
    setStatus('status_searching');
    addLog(t('log_searching'));
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: 'OpenNerfESC' }],
      optionalServices: [SERVICE_UUID]
    });
    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);
    setStatus('status_connecting');
    addLog(t('log_connecting'));
    const server  = await bleDevice.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    chars.spinUpTime  = await service.getCharacteristic(CHAR_SPIN_UP_TIME);
    chars.targetSpeed = await service.getCharacteristic(CHAR_TARGET_SPEED);
    chars.minVoltage  = await service.getCharacteristic(CHAR_MIN_VOLTAGE);
    // Read current values
    try {
      const sv = (await chars.spinUpTime.readValue()).getUint16(0, true);
      const sp = (await chars.targetSpeed.readValue()).getUint8(0);
      const vo = (await chars.minVoltage.readValue()).getFloat32(0, true);
      inputSpin.value    = sv;  spinValue.textContent    = sv + ' ms';
      inputSpeed.value   = sp;  speedValue.textContent   = sp + '%';
      inputVoltage.value = vo.toFixed(1); voltageValue.textContent = vo.toFixed(1) + ' V';
      addLog(t('log_read', sv, sp, vo.toFixed(1)));
    } catch(e) { addLog(t('log_read_err') + e.message); }
    setStatusText(t('log_connected') + bleDevice.name, 'connected');
    btnConnect.textContent = t('btn_connect_active');
    btnSend.disabled = false;
    addLog(t('log_connected') + bleDevice.name);
  } catch(e) {
    setStatus('status_conn_err', 'error');
    addLog(t('log_conn_err') + e.message);
  }
});

function onDisconnected() {
  setStatusText(t('disconnected'), 'error');
  btnConnect.textContent = t('btn_connect');
  btnSend.disabled = true;
  chars = {};
  addLog(t('log_disconnected'));
}

// --- Send all parameters at once ---
btnSend.addEventListener('click', async () => {
  const sv = parseInt(inputSpin.value, 10);
  const sp = parseInt(inputSpeed.value, 10);
  const vo = parseFloat(inputVoltage.value);
  if (isNaN(sv) || sv < 0   || sv > 500)  { addLog(t('err_spin'));  return; }
  if (isNaN(sp) || sp < 0   || sp > 100)  { addLog(t('err_speed')); return; }
  if (isNaN(vo) || vo < 3.0 || vo > 15.0) { addLog(t('err_volt'));  return; }
  try {
    await chars.spinUpTime.writeValueWithResponse(uint16LE(sv));
    await chars.targetSpeed.writeValueWithResponse(uint8Buf(sp));
    await chars.minVoltage.writeValueWithResponse(float32LE(vo));
    addLog(t('log_sent', sv, sp, vo.toFixed(1)));
  } catch(e) {
    addLog(t('log_write_err') + e.message);
    setStatus('status_write_err', 'error');
  }
});

// Apply language on load
applyLang();
