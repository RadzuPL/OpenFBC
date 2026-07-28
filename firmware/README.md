# Firmware — OpenFBC

> **Język / Language:** [🇵🇱 Polski](#-opis) · [🇬🇧 English](#-description)

---

## 🇵🇱 Opis

Kod źródłowy firmware dla ESP32-C3/C6 oparty na Arduino Core + PlatformIO. Firmware obsługuje sterowanie PWM silnikami flywheel, logikę spin-up/cruise, pomiar napięcia akumulatora oraz konfigurację przez BLE (GATT).

> **Nie chcesz kompilować?** Skorzystaj z gotowego firmware przez Web Flasher: [flasher.radzu.net](https://flasher.radzu.net)

### Środowisko

- **Visual Studio Code** z rozszerzeniem **PlatformIO IDE**
- Repozytorium klonujesz lokalnie, otwierasz folder `firmware/` jako projekt PlatformIO

### Środowiska budowania (`platformio.ini`)

| Środowisko | Płytka | Przeznaczenie |
|---|---|---|
| `esp32c3-prod` | ESP32-C3 Super Mini | Docelowy hardware (PowerBoard v1) |
| `esp32c6-test` | ESP32-C6 Super Mini (custom 4MB/FH4) | Platforma testowa |

### Architektura kodu

| Plik | Opis |
|---|---|
| `src/main.cpp` | Punkt wejścia — inicjalizacja pinów, przerwań i modułów |
| `src/ble_server.cpp/.h` | Serwer GATT BLE — UUID, charakterystyki, obsługa połączeń |
| `src/battery_monitor.cpp/.h` | Odczyt napięcia baterii i buforowany pomiar spoczynkowy do blokady low-voltage |
| `src/pwm_control.cpp/.h` | Logika LEDC PWM — spin-up, cruise, odczyt triggera i przypomnienia silnikami |
| `src/params.cpp/.h` | Parametry w NVS (Preferences) — odczyt/zapis przez BLE i PWM |
| `include/config.h` | Stałe konfiguracyjne — piny, częstotliwość PWM, zakresy |
| `platformio.ini` | Konfiguracja środowisk i zależności |

### Mapowanie pinów ESP32-C3 Super Mini

| Sygnał | Pin | Opis |
|---|---|---|
| PWM (oba silniki) | GPIO 2 | Jeden pin → TC4420 → MOSFET → silniki równolegle |
| Spust (Trigger) | GPIO 4 | Active LOW, wewnętrzny pull-up — zwierany do GND |
| ADC napięcia baterii | GPIO 1 | Dzielnik napięcia LiPo |

### Logika sterowania silnikami

Silniki flywheel potrzebują krótkiego impulsu pełnej mocy, żeby szybko dobiec do prędkości roboczej. Sekwencja po naciśnięciu spustu:

```
Spust wciśnięty:
  Faza spin-up:         PWM = 100%              przez czas spin-up [ms]
                        (tylko jeśli spust był puszczony przez min. czas blokady re-spin-up [ms])
  Szybki re-trigger:    PWM = 100%              przez czas szybkiego re-trigger [ms]
                        (gdy czas blokady re-spin-up jeszcze nie minął)
  Faza cruise:          PWM = prędkość robocza  (do zwolnienia spustu)

Spust zwolniony:
  Natychmiastowy stop:  PWM = 0%
```

> Nazwy parametrów odpowiadają etykietom w Web Konfiguratorze ([one.radzu.net](https://one.radzu.net)).

Blokada minimalnego napięcia korzysta z ostatniego pomiaru wykonanego po co najmniej 5 s bezczynności silników, żeby nie reagować na chwilowe spadki pod obciążeniem. Po 1 godzinie od uruchomienia lub ostatniego wciśnięcia spustu firmware odtwarza krótki sygnał silnikami i ponawia go co 5 minut, dopóki blaster pozostaje nieużywany.

### Automatyczny build (GitHub Actions)

Workflow `.github/workflows/firmware-build.yml` uruchamia się przy push do `firmware/**` lub przy tworzeniu Release. Buduje pliki `.bin` dla obu środowisk i dołącza je jako assets do Release.

---

## 🇬🇧 Description

Firmware source code for ESP32-C3/C6 based on Arduino Core + PlatformIO. Handles flywheel motor PWM control, spin-up/cruise logic, battery voltage monitoring and BLE configuration (GATT).

> **Don't want to compile?** Use the prebuilt firmware via Web Flasher: [flasher.radzu.net](https://flasher.radzu.net)

### Environment

- **Visual Studio Code** with **PlatformIO IDE** extension
- Clone the repo locally and open the `firmware/` folder as a PlatformIO project

### Build Environments (`platformio.ini`)

| Environment | Board | Purpose |
|---|---|---|
| `esp32c3-prod` | ESP32-C3 Super Mini | Target hardware (PowerBoard v1) |
| `esp32c6-test` | ESP32-C6 Super Mini (custom 4MB/FH4) | Test platform |

### Code Architecture

| File | Description |
|---|---|
| `src/main.cpp` | Entry point — pin init, interrupts, module setup |
| `src/ble_server.cpp/.h` | BLE GATT server — UUIDs, characteristics, connections |
| `src/battery_monitor.cpp/.h` | Battery voltage reads and cached idle-only sampling for low-voltage cutoff |
| `src/pwm_control.cpp/.h` | LEDC PWM logic — spin-up, cruise, trigger reading and motor reminders |
| `src/params.cpp/.h` | Parameters in NVS (Preferences) — read/write via BLE and PWM |
| `include/config.h` | Compile-time constants — pins, PWM frequency, parameter ranges |
| `platformio.ini` | Environment and dependency configuration |

### ESP32-C3 Super Mini Pin Mapping

| Signal | Pin | Description |
|---|---|---|
| PWM (both motors) | GPIO 2 | Single pin → TC4420 → MOSFET → motors in parallel |
| Trigger | GPIO 4 | Active LOW, internal pull-up — connect to GND |
| Battery voltage ADC | GPIO 1 | LiPo voltage divider |

### Motor Control Logic

Flywheel motors need a brief full-power burst to quickly reach operating speed. Sequence after trigger press:

```
Trigger pressed:
  Spin-up phase:        PWM = 100%            for spin-up time [ms]
                        (only if trigger was released for at least re-spin-up lock time [ms])
  Quick re-trigger:     PWM = 100%            for quick re-trigger time [ms]
                        (when re-spin-up lock time has not yet elapsed)
  Cruise phase:         PWM = cruise speed    (until trigger release)

Trigger released:
  Immediate stop:       PWM = 0%
```

> Parameter names match the labels shown in the Web Configurator ([one.radzu.net](https://one.radzu.net)).

The minimum-voltage cutoff uses the latest sample collected only after at least 5 seconds of motor inactivity, which avoids false triggers from temporary voltage sag under load. After 1 hour since power-on or the last trigger press, firmware plays a short reminder on the motors and repeats it every 5 minutes while the blaster stays unused.

### Automated Build (GitHub Actions)

Workflow `.github/workflows/firmware-build.yml` triggers on push to `firmware/**` or on Release creation. Builds `.bin` files for both environments and attaches them as Release assets.
