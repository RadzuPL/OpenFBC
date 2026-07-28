# OpenFBC — *Open Flywheel Blaster Controller*

> **Język / Language:** [🇵🇱 Polski](#-opis-projektu) · [🇬🇧 English](#-project-description)

---

## 🇵🇱 Opis projektu

OpenFBC to otwartoźródłowy, miniaturowy i bezdźwiękowy (PWM 20 kHz+) sterownik silników Flywheel do wyrzutni strzałkowych. Konfiguracja odbywa się bezprzewodowo przez Bluetooth Low Energy (BLE) z poziomu przeglądarki — bez rozkręcania wyrzutni, bez instalowania aplikacji.

### Jak zacząć?

1. **Wgraj firmware** na swój ESP32-C3 Super Mini korzystając z Web Flashera:
   👉 **[flasher.radzu.net](https://flasher.radzu.net)** — wystarczy Chrome lub Edge i kabel USB.

2. **Zamontuj płytkę** PowerBoard v1 w wyrzutni i podłącz silniki, zasilanie i spust.

3. **Skonfiguruj** parametry pracy (prędkość, spin-up, ochrona baterii) przez Web Konfigurator:
   👉 **[one.radzu.net](https://one.radzu.net)** — działa z poziomu przeglądarki przez Bluetooth.

> **iOS/Safari:** Web Bluetooth nie jest natywnie wspierany. Użyj aplikacji **[Bluefy](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055)**.

### Parametry konfiguracyjne

Wszystkie parametry ustawia się w Web Konfiguratorze ([one.radzu.net](https://one.radzu.net)):

| Parametr w konfiguratorze | Zakres | Domyślnie | Opis |
|---|---|---|---|
| Czas spin-up | 0 – 500 ms | 200 ms | Czas pełnej mocy (100% PWM) przy starcie silników. Wartość 0 = brak spin-up. |
| Czas blokady re-spin-up | 0 – 5000 ms | 3000 ms | Minimalny czas zwolnienia spustu przed kolejnym pełnym spin-up. |
| Czas szybkiego re-trigger | 0 – 100 ms | 50 ms | Krótki spin-up przy szybkim ponownym naciśnięciu spustu. |
| Prędkość robocza | 0 – 100 % | 75 % | Wypełnienie PWM po fazie spin-up (prędkość cruise). |
| Minimalne napięcie | 3.0 – 15.0 V | 11.1 V | Próg ochrony akumulatora — blokada strzału działa na podstawie pomiaru wykonanego po 5 s postoju silników. |

Konfigurator wyświetla też aktualne napięcie baterii w czasie rzeczywistym.

Po 1 godzinie od uruchomienia lub ostatniego wciśnięcia spustu firmware przypomina o wyłączeniu nieużywanego blastera krótkim sygnałem silnikami, a następnie powtarza przypomnienie co 5 minut.

### Obsługiwane płytki

| Płytka | Status |
|---|---|
| ESP32-C3 Super Mini | ✅ Docelowy hardware |
| ESP32-C6 Super Mini | 🧪 Platforma testowa |

### Struktura repozytorium

| Katalog | Zawartość |
|---|---|
| [`/hardware`](hardware/README.md) | Schemat, BOM, pliki Gerber, wizualizacje PCB dla PowerBoard v1 |
| [`/firmware`](firmware/README.md) | Kod źródłowy ESP32 (PlatformIO), instrukcja budowania i wgrywania |
| [`/web-config`](web-config/README.md) | Konfigurator webowy (BLE), obraz Docker, opis wdrożenia |
| [`/web-flasher`](web-flasher/README.md) | Web Flasher (ESP Web Tools), obraz Docker, opis wdrożenia |
| [`/docs`](docs/) | Dodatkowa dokumentacja i materiały |

### Znane ograniczenia

- **iOS/Safari** nie wspiera Web Bluetooth — wymagana aplikacja Bluefy.
- **Web Flasher** działa wyłącznie w Chrome i Edge (WebSerial API).

### TODO

#### Hardware / elektronika
- [x] Zamówić płytki PCB PowerBoard v1.
- [x] Zamówić wszystkie komponenty elektroniczne.
- [ ] Zlutować i uruchomić pierwsze prototypy PowerBoard v1.
- [ ] Zweryfikować termikę i stabilność sekcji mocy pod obciążeniem.

#### Firmware (ESP32-C3)
- [x] Implementacja odczytu ADC napięcia i blokady przy minimalnym napięciu.
- [ ] Dokończyć implementację wszystkich trybów sterowania PWM.
- [ ] Testy na docelowym ESP32-C3 Super Mini (aktualnie: ESP32-C6 DevKitC-1).
- [ ] Podnieść częstotliwość PWM do 20 kHz po montażu TC4420.
- [ ] Przetestować komunikację BLE i kompatybilność z różnymi telefonami.

#### Web Config
- [x] Pola minVoltage, spinUpRearmTime i reTriggerSpinUpTime w UI.
- [x] Odczyt napięcia baterii (read-only) w konfiguratorze.
- [ ] Dopracować UI i walidację parametrów.
- [ ] Sprawdzić stabilność połączenia Web Bluetooth i obsługę błędów.

#### Web Flasher
- [x] Kontener Docker opublikowany na GHCR.
- [x] Flasher dostępny pod `flasher.radzu.net`.
- [x] Automatyczny build firmware w GitHub Actions (ESP32-C3 + C6).
- [x] Pierwszy Release z tagiem — `.bin` w assets.
- [ ] Test end-to-end: Release → flasher → fizyczne ESP32.
- [ ] Wyświetlanie aktualnej wersji firmware na stronie flashera.

#### Testy
- [ ] Weryfikacja działania Spin-Up/Cruise na fizycznym silniku.
- [ ] Test end-to-end Web Flasher: wgranie firmware przez przeglądarkę.

#### Dokumentacja
- [ ] Uzupełnić README po każdym większym etapie.

#### Infrastruktura
- [ ] Migracja PlatformIO z `pioarduino` na oficjalny `espressif32` (gdy Arduino Core 3.x trafi do stable registry).

### Licencja

Projekt jest udostępniany na licencji **GNU Affero General Public License v3.0 (AGPL-3.0)**.
Pełny tekst: [`/LICENSE`](LICENSE).

---

## 🇬🇧 Project Description

OpenFBC is an open-source, compact and silent (PWM 20 kHz+) flywheel motor controller for dart blasters. Configuration is done wirelessly via Bluetooth Low Energy (BLE) directly from a browser — no disassembly, no app installation required.

### Getting Started

1. **Flash the firmware** to your ESP32-C3 Super Mini using the Web Flasher:
   👉 **[flasher.radzu.net](https://flasher.radzu.net)** — requires Chrome or Edge and a USB cable.

2. **Install the board** (PowerBoard v1) in your blaster and connect motors, power and trigger.

3. **Configure** operating parameters (speed, spin-up timing, battery protection) via the Web Configurator:
   👉 **[one.radzu.net](https://one.radzu.net)** — works in-browser over Bluetooth.

> **iOS/Safari:** Web Bluetooth is not natively supported. Use the **[Bluefy](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055)** app.

### Configuration Parameters

All parameters are set in the Web Configurator ([one.radzu.net](https://one.radzu.net)):

| Parameter | Range | Default | Description |
|---|---|---|---|
| Spin-up time | 0 – 500 ms | 200 ms | Full power (100% PWM) duration when motors start. 0 = no spin-up phase. |
| Re-spin-up lock time | 0 – 5000 ms | 3000 ms | Minimum trigger release time before a full spin-up is allowed again. |
| Quick re-trigger time | 0 – 100 ms | 50 ms | Short spin-up used when trigger is pressed again before lock expires. |
| Cruise speed | 0 – 100 % | 75 % | PWM duty cycle after spin-up (running speed). |
| Minimum voltage | 3.0 – 15.0 V | 11.1 V | Battery protection threshold — blaster is disabled below this voltage. |

The configurator also shows current battery voltage in real time.

### Supported Boards

| Board | Status |
|---|---|
| ESP32-C3 Super Mini | ✅ Target hardware |
| ESP32-C6 Super Mini | 🧪 Test platform |

### Repository Structure

| Directory | Contents |
|---|---|
| [`/hardware`](hardware/README.md) | Schematic, BOM, Gerber files, PCB renders for PowerBoard v1 |
| [`/firmware`](firmware/README.md) | ESP32 source code (PlatformIO), build and flash instructions |
| [`/web-config`](web-config/README.md) | BLE Web Configurator, Docker image, deployment notes |
| [`/web-flasher`](web-flasher/README.md) | Web Flasher (ESP Web Tools), Docker image, deployment notes |
| [`/docs`](docs/) | Additional documentation and materials |

### Known Limitations

- **iOS/Safari** does not support Web Bluetooth — Bluefy app required.
- **Web Flasher** works only in Chrome and Edge (WebSerial API).

### License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.
Full text: [`/LICENSE`](LICENSE).
