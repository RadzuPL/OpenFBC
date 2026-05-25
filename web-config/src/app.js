// =============================================================================
// OpenNerfESC - app.js
// Web Bluetooth client. Connects to ESP32-C6/C3 BLE server and writes
// spinUpTime, targetSpeed and minVoltage characteristics.
// UUIDs must match config.h on the firmware side.
// =============================================================================

'use strict';

// --- BLE UUIDs (must match firmware/include/config.h) ---
const SERVICE_UUID       = '12345678-1234-1234-1234-123456789abc';
const CHAR_SPIN_UP_TIME  = '12345678-1234-1234-1234-123456789001';
const CHAR_TARGET_SPEED  = '12345678-1234-1234-1234-123456789002';
const CHAR_MIN_VOLTAGE   = '12345678-1234-1234-1234-123456789003';

// --- DOM refs ---
const btnConnect    = document.getElementById('btn-connect');
const btnSend       = document.getElementById('btn-send');
const statusBadge   = document.getElementById('status');
const inputSpin     = document.getElementById('input-spin-up-time');
const inputSpeed    = document.getElementById('input-target-speed');
const inputVoltage  = document.getElementById('input-min-voltage');
const spinValue     = document.getElementById('spin-value');
const speedValue    = document.getElementById('speed-value');
const voltageValue  = document.getElementById('voltage-value');
const log           = document.getElementById('log');

// --- State ---
let bleDevice = null;
let chars = {};

// --- Helpers ---
function setStatus(text, cls) {
  statusBadge.textContent = text;
  statusBadge.className = 'status ' + (cls || '');
}

function addLog(msg) {
  const line = document.createElement('div');
  line.textContent = new Date().toLocaleTimeString() + '  ' + msg;
  log.prepend(line);
  if (log.children.length > 40) log.lastChild.remove();
}

function uint16LE(value) {
  const buf = new ArrayBuffer(2);
  new DataView(buf).setUint16(0, value, true);
  return buf;
}

function uint8Buf(value) {
  const buf = new ArrayBuffer(1);
  new DataView(buf).setUint8(0, value);
  return buf;
}

function float32LE(value) {
  const buf = new ArrayBuffer(4);
  new DataView(buf).setFloat32(0, value, true);
  return buf;
}

// --- Slider live value display ---
inputSpin.addEventListener('input', () => spinValue.textContent = inputSpin.value + ' ms');
inputSpeed.addEventListener('input', () => speedValue.textContent = inputSpeed.value + '%');
inputVoltage.addEventListener('input', () => voltageValue.textContent = parseFloat(inputVoltage.value).toFixed(1) + ' V');

// --- Connect ---
btnConnect.addEventListener('click', async () => {
  if (!navigator.bluetooth) {
    alert('Ta przeglądarka nie obsługuje Web Bluetooth. Użyj Chrome/Edge na desktopie.');
    return;
  }
  if (bleDevice && bleDevice.gatt.connected) {
    bleDevice.gatt.disconnect();
    return;
  }
  try {
    setStatus('Szukam urządzenia...', '');
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: 'OpenNerfESC' }],
      optionalServices: [SERVICE_UUID]
    });
    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);

    setStatus('Łączę...', '');
    const server  = await bleDevice.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);

    chars.spinUpTime  = await service.getCharacteristic(CHAR_SPIN_UP_TIME);
    chars.targetSpeed = await service.getCharacteristic(CHAR_TARGET_SPEED);
    chars.minVoltage  = await service.getCharacteristic(CHAR_MIN_VOLTAGE);

    // Read current values from device
    try {
      const spinRaw  = await chars.spinUpTime.readValue();
      const speedRaw = await chars.targetSpeed.readValue();
      const voltRaw  = await chars.minVoltage.readValue();
      const spinVal  = spinRaw.getUint16(0, true);
      const speedVal = speedRaw.getUint8(0);
      const voltVal  = voltRaw.getFloat32(0, true);
      inputSpin.value    = spinVal;
      inputSpeed.value   = speedVal;
      inputVoltage.value = voltVal.toFixed(1);
      spinValue.textContent    = spinVal + ' ms';
      speedValue.textContent   = speedVal + '%';
      voltageValue.textContent = voltVal.toFixed(1) + ' V';
      addLog('Odczytano wartości: spinUp=' + spinVal + 'ms speed=' + speedVal + '% volt=' + voltVal.toFixed(1) + 'V');
    } catch(e) {
      addLog('Nie udało się odczytać wartości: ' + e.message);
    }

    setStatus('Połączono z ' + bleDevice.name, 'connected');
    btnConnect.textContent = 'Rozłącz';
    btnSend.disabled = false;
    addLog('Połączono z ' + bleDevice.name);
  } catch (e) {
    setStatus('Błąd połączenia', 'error');
    addLog('Błąd: ' + e.message);
  }
});

function onDisconnected() {
  setStatus('Rozłączono', 'error');
  btnConnect.textContent = 'Połącz z ESC';
  btnSend.disabled = true;
  chars = {};
  addLog('Urządzenie rozłączone');
}

// --- Send parameters ---
btnSend.addEventListener('click', async () => {
  const spinVal  = parseInt(inputSpin.value,  10);
  const speedVal = parseInt(inputSpeed.value, 10);
  const voltVal  = parseFloat(inputVoltage.value);

  if (isNaN(spinVal)  || spinVal  < 0   || spinVal  > 5000) { addLog('spinUpTime poza zakresem (0-5000)');  return; }
  if (isNaN(speedVal) || speedVal < 0   || speedVal > 100)  { addLog('targetSpeed poza zakresem (0-100)');   return; }
  if (isNaN(voltVal)  || voltVal  < 3.0 || voltVal  > 25.0) { addLog('minVoltage poza zakresem (3.0-25.0)'); return; }

  try {
    await chars.spinUpTime.writeValueWithResponse(uint16LE(spinVal));
    await chars.targetSpeed.writeValueWithResponse(uint8Buf(speedVal));
    await chars.minVoltage.writeValueWithResponse(float32LE(voltVal));
    addLog('Wysłano: spinUp=' + spinVal + 'ms speed=' + speedVal + '% volt=' + voltVal.toFixed(1) + 'V');
  } catch(e) {
    addLog('Błąd zapisu: ' + e.message);
    setStatus('Błąd zapisu', 'error');
  }
});
