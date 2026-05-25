const connectBtn = document.getElementById('connect');
const dutyInput = document.getElementById('duty');
const dutyValue = document.getElementById('duty-value');

connectBtn?.addEventListener('click', async () => {
  if (!navigator.bluetooth) {
    alert('Przeglądarka nie wspiera Web Bluetooth API.');
    return;
  }

  try {
    await navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: 'OpenNerfESC' }],
      optionalServices: ['0000ffe0-0000-1000-8000-00805f9b34fb']
    });
  } catch (error) {
    console.error('Nie udało się połączyć:', error);
  }
});

dutyInput?.addEventListener('input', () => {
  dutyValue.textContent = dutyInput.value;
});
