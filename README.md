# OpenNerfESC

## ⚠️ Status projektu
Projekt jest na wczesnym etapie i ciągle trwają nad nim prace.

## ✅ Full release (aktualny stan)
- Firmware kompiluje się w **Visual Studio Code + PlatformIO**.
- Ustawienia można edytować z poziomu strony konfiguracyjnej.
- ⚠️ **Autobuild może jeszcze nie działać poprawnie w każdym przypadku.**

## Licencja
Projekt jest udostępniany na licencji **GNU Affero General Public License v3.0 (AGPL-3.0)**.
Pełny tekst licencji znajduje się w pliku `/LICENSE`.

## 1. Wstęp i założenia projektu
OpenNerfESC to otwartoźródłowy, miniaturowy i bezdźwiękowy (20kHz+) sterownik PWM do silników szczotkowych Flywheel w wyrzutniach strzałkowych. Sterowanie odbywa się z poziomu przeglądarki przez Web Bluetooth API, więc nie trzeba rozkręcać wyrzutni i kręcić potencjometrem.

## 2. Hardware (Elektronika i „Kanapka")
 - **Mózg:** TENSTAR ESP32-C3 Super Mini – https://pl.aliexpress.com/item/1005009890133886.html
 - **Driver bramki MOSFET:** TC4420 – https://pl.aliexpress.com/item/1005011629643514.html
 - **Element wykonawczy:** N-MOSFET IRLR7843 (TO-252) – https://pl.aliexpress.com/item/1005006127790007.html
 - **Zabezpieczenia:** dioda Schottky'ego SB540 5A40V – https://pl.aliexpress.com/item/1005007048222899.html
 - **Zasilanie logiki:** Mini DC-DC 12-24V do 5V 3A – https://pl.aliexpress.com/item/1005006245122273.html
   (na płytce prawdopodobnie MP2315 z elementami dodatkowymi; moduł będzie wylutowany i potrzebne elementy trafią na docelową płytkę)
 - **Połączenia:** XT30 i solidne ścieżki z odniesieniem do `/hardware/wiring_diagram.png` (wpięcie oryginalnego Rev Triggera jako wejście logiczne ESP32).
 - **Silniki:** dwa szczotkowe silniki DC połączone równolegle, sterowane jednym kanałem PWM (jeden pin ESP32 → jeden MOSFET).

Struktura folderu sprzętowego:
 - `/hardware/gerbers` – paczki produkcyjne PCB (ZIP do JLCPCB).
 - `/hardware/schematics` – zrzuty schematów z EasyEDA.
 - `/hardware/wiring_diagram.png` – diagram połączeń całej instalacji.

## 3. Logika sterowania silnikami

Silniki flywheel wymagają krótkiego impulsu pełnej mocy przy starcie, aby przełamać bezwładność i szybko dobiec do prędkości roboczej. Dlatego sekwencja działania po naciśnięciu spustu jest następująca:

```
Trigger wciśnięty:
  Faza 1 (Spin-Up):   PWM = 100%   przez czas spinUpTime [ms]
                      (tylko jeśli spust był puszczony przez min. spinUpRearmTime [ms])
  Szybki re-trigger:  PWM = 100%   przez czas reTriggerSpinUpTime [ms]
                      (gdy spinUpRearmTime jeszcze nie minął)
  Faza 2 (Cruise):    PWM = targetSpeed [%]   (aż do zwolnienia spustu)

Trigger zwolniony:
  Natychmiastowy stop: PWM = 0%
  Ponowny pełny Spin-Up odblokowuje się dopiero po spinUpRearmTime ms bez wciskania spustu
```

### Parametry konfiguracyjne (ustawiane przez BLE / Web Config)

| Parametr | Zakres | Domyślnie | Opis |
|---|---|---|---|
| `spinUpTime` | 0 – 500 ms | 200 ms | Czas trwania fazy Spin-Up na pełnej mocy (100% PWM). Ustawienie 0 pomija fazę Spin-Up i od razu przechodzi do Cruise. |
| `spinUpRearmTime` | 0 – 5000 ms | 3000 ms | Minimalny czas zwolnienia spustu wymagany do ponownego uruchomienia pełnego Spin-Up. |
| `reTriggerSpinUpTime` | 0 – 100 ms | 50 ms | Krótki Spin-Up używany przy szybkim ponownym naciśnięciu spustu zanim minie `spinUpRearmTime`. Wartość jest ograniczona do `<= spinUpTime`. |
| `targetSpeed` | 0 – 100 % | 75 % | Wypełnienie PWM w fazie Cruise — prędkość robocza silników podczas włączonego spustu. |
| `minVoltage` | 3.0 – 15.0 V | 11.1 V | Minimalne napięcie pakietu LiPo. Poniżej tej granicy wyrzutnia jest blokowana (ochrona akumulatora). |

Konfigurator Web Config odczytuje też aktualne napięcie baterii (read-only) przez BLE, aby umożliwić kalibrację i weryfikację dzielnika napięcia.

### Przykład dla domyślnych ustawień

```
Spust wciśnięty → 0–200ms: 100% PWM (spin-up)
              200ms+:  75% PWM  (cruise, silniki kręcą z roboczą prędkością)
Spust zwolniony → natychmiastowy stop
```

## 4. Firmware (Oprogramowanie układowe mikrokontrolera)
Wymagane środowisko: **Visual Studio Code + PlatformIO**.

Architektura kodu firmware:
 - `firmware/src/main.cpp` – spina moduły, inicjuje piny i przerwania.
 - `firmware/src/ble_server.cpp` + `firmware/src/ble_server.h` – serwer GATT, UUID i obsługa parametrów.
 - `firmware/src/pwm_control.cpp` + `firmware/src/pwm_control.h` – obsługa LEDC, logika Spin-Up/Cruise, odczyt triggera.
 - `firmware/src/params.cpp` + `firmware/src/params.h` – przechowywanie parametrów w NVS (Preferences), dostęp z BLE i PWM.
 - `firmware/include/config.h` – twardo zdefiniowane stałe (piny, częstotliwość PWM, zakresy parametrów).
 - `firmware/platformio.ini` – konfiguracja środowiska i bibliotek.

### Piny ESP32-C3 Super Mini

| Sygnał | Pin | Opis |
|---|---|---|
| PWM (oba silniki) | GPIO 2 | Jeden pin → MOSFET → silniki równolegle |
| Trigger (spust) | GPIO 4 | Wejście logiczne, active LOW, z wewnętrznym pull-up (przycisk do masy) |
| ADC napięcia | GPIO 1 | Dzielnik napięcia LiPo |

## 5. Web Config (Interfejs sterowania i konteneryzacja)
Interfejs to statyczna strona HTML/JS korzystająca z `navigator.bluetooth` — bez frameworków i bez `npm install` dla samego frontu.

 - Kod strony: `web-config/src/index.html`, `web-config/src/style.css`, `web-config/src/app.js`.
 - Konteneryzacja: `web-config/Dockerfile` oparty o `nginx:alpine`.
 - Automatyzacja: `.github/workflows/docker-publish.yml` buduje i publikuje obraz na GHCR przy zmianach w `web-config/`.
 - Publiczny interfejs dostępny pod adresem: **https://one.radzu.net**

Przykładowy flow wdrożenia:
1. Push zmian webowych do repo.
2. GitHub Actions buduje i publikuje nowy obraz (tag `latest` + `main` + SHA commita).
3. Watchtower (lub inny agent) na serwerze odświeża kontener automatycznie.
4. Nowa wersja dostępna pod adresem `one.radzu.net`.

Szybki start (Docker):
```yaml
services:
  opennerfesc-web:
    image: ghcr.io/radzupl/opennerfesc-web-config:latest
    container_name: opennerfesc-web
    restart: unless-stopped
    ports:
      - "8321:80"
```

## 6. Web Flasher (Wgrywanie firmware przez przeglądarkę)
Web Flasher pozwala wgrać firmware na ESP32 bezpośrednio przez przeglądarkę (Chrome/Edge) bez instalacji żadnych narzędzi — wystarczy kabel USB.

 - Publiczny flasher dostępny pod adresem: **https://flasher.radzu.net**
 - Technologia: [ESP Web Tools](https://esphome.github.io/esp-web-tools/) — ta sama biblioteka co ESPHome/Tasmota.
 - Kod strony: `web-flasher/src/index.html`, `web-flasher/manifests/manifest-esp32c3.json`, `web-flasher/manifests/manifest-esp32c6.json`.
 - Konteneryzacja: `web-flasher/Dockerfile` oparty o `nginx:alpine`, obraz: `ghcr.io/radzupl/opennerfesc-flasher:latest`.
 - Automatyzacja buildu firmware: `.github/workflows/firmware-build.yml` — buduje `.bin` dla obu płytek przy push do `firmware/**` lub przy tworzeniu Release.

### Obsługiwane płytki

| Płytka | Środowisko PlatformIO | Status |
|---|---|---|
| ESP32-C3 Super Mini | `esp32c3-prod` | Docelowy hardware |
| ESP32-C6 Super Mini | `esp32c6-test` | Platforma testowa (custom board 4MB / FH4) |

### Flow flashowania
1. Utwórz **Release** w GitHub → `firmware-build.yml` buduje `.bin` i dołącza do Release assets.
2. Wejdź na **https://flasher.radzu.net** w Chrome lub Edge (wymagane HTTPS + WebSerial API).
3. Podłącz ESP32 kablem USB, kliknij przycisk odpowiedniej płytki — przeglądarka wgrywa firmware automatycznie.

> **Uwaga:** WebSerial API działa tylko w Chrome i Edge. Firefox i Safari nie są wspierane.

Szybki start (Docker):
```yaml
services:
  opennerfesc-flasher:
    image: ghcr.io/radzupl/opennerfesc-flasher:latest
    container_name: OpenNerfESC-Flasher
    restart: unless-stopped
    ports:
      - "8322:8080"
```

## 7. Znane problemy
 - iOS/Safari nie wspiera natywnie Web Bluetooth. Użytkownicy Apple powinni używać aplikacji **Bluefy**.
 - WebSerial API (Web Flasher) nie działa w Firefox ani Safari — wymagany Chrome lub Edge.

## 8. TODO (z podziałem na obszary projektu)

### Hardware / elektronika
 - [ ] Dokończyć i zweryfikować pełny schemat elektryczny.
 - [ ] Narysować finalne PCB pod docelowe elementy (w tym elementy z modułu MP2315) – IRLR7843 + TC4420 + SB540 + zasilacz 5V.
 - [ ] Wykonać ERC/DRC i przegląd obciążalności ścieżek mocy.
 - [ ] Zamówić IRLR7843 (oczekiwanie na dostawę) i prototypowe płytki PCB.
 - [ ] Zlutować i uruchomić pierwsze prototypy.
 - [ ] Zweryfikować termikę i stabilność sekcji mocy pod obciążeniem.

### Firmware (ESP32-C3)
 - [ ] Implementacja odczytu ADC napięcia i blokady przy `minVoltage`.
 - [ ] Dokończyć implementację i konfigurację wszystkich trybów sterowania PWM.
 - [ ] Sprawdzić i dostroić parametry bezpieczeństwa (limity, stany awaryjne).
 - [ ] Przetestować komunikację BLE i kompatybilność z różnymi telefonami.
 - [ ] Testy na docelowym ESP32-C3 Super Mini (aktualnie testy na ESP32-C6 DevKitC-1).
 - [ ] Podnieść `PWM_FREQ_HZ` do 20 kHz po montażu TC4420 (aktualnie 4 kHz — celowo na czas testów z BJT gate-driverem).

### Web Config / aplikacja webowa
 - [x] Dodanie pól minVoltage, spinUpRearmTime i reTriggerSpinUpTime do UI.
 - [x] Dodanie odczytu napięcia baterii (read-only) w konfiguratorze.
 - [ ] Dopracować UI i walidację parametrów.
 - [ ] Sprawdzić stabilność połączenia Web Bluetooth i obsługę błędów.
 - [ ] Uzupełnić dokumentację użytkownika dla konfiguratora.

### Web Flasher
 - [x] Skonfigurować kontener Docker (`nginx:alpine`) i opublikować obraz na GHCR.
 - [x] Wdrożyć flasher pod adresem `https://flasher.radzu.net` (Cloudflare Tunnel + SSL Flexible rule).
 - [x] Zaimplementować automatyczny build firmware w GitHub Actions (matrix build: C3 + C6).
 - [x] Manifesty ESP Web Tools dla obu płytek wskazujące na GitHub Release assets.
 - [x] Wykonać pierwszy Release z tagiem i zweryfikować że `.bin` pojawia się w assets.
 - [ ] Przetestować end-to-end: Release → flasher → fizyczne ESP32.
 - [ ] Dodać wyświetlanie aktualnej wersji firmware na stronie flashera (GitHub Releases API).

### Konteneryzacja i deployment
 - [x] Skonfigurować kontener Docker dla web-config (`nginx:alpine`).
 - [x] Zweryfikować automatyczny build/publish obrazu w GitHub Actions (tag `latest` na branchu `main`).
 - [x] Obraz publiczny dostępny na GHCR: `ghcr.io/radzupl/opennerfesc-web-config:latest`.
 - [x] Strona placeholder działa i jest dostępna publicznie pod `one.radzu.net`.
 - [x] Dopiąć subdomenę do kontenera i potwierdzić dostępność po HTTPS (SSL przez Cloudflare).
 - [x] Skonfigurować kontener i subdomenę `flasher.radzu.net` dla Web Flashera.

### Testy
 - [ ] Weryfikacja działania Spin-Up/Cruise na fizycznym silniku.
 - [ ] Test end-to-end Web Flasher: wgranie firmware przez przeglądarkę na ESP32-C3.

### Organizacja projektu i dokumentacja
 - [ ] Uporządkować backlog i kolejność prac (MVP → kolejne iteracje).
 - [ ] Uzupełnić README o status postępu po każdym większym etapie.

### Zależności i infrastruktura
 - [ ] Migracja platformy PlatformIO z `pioarduino` (fork) na oficjalny `espressif32` gdy Arduino Core 3.x trafi do stabilnego PlatformIO registry — dotyczy obu środowisk (`esp32c3-prod` i `esp32c6-test`).
