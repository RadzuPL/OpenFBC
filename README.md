# OpenNerfESC

## 1. Wstęp i założenia projektu
OpenNerfESC to otwartoźródłowy, miniaturowy i bezdźwiękowy (20kHz+) sterownik PWM do silników szczotkowych Flywheel w wyrzutniach strzałkowych. Sterowanie odbywa się z poziomu przeglądarki przez Web Bluetooth API, więc nie trzeba rozkręcać wyrzutni i kręcić potencjometrem.

## 2. Hardware (Elektronika i „Kanapka”)
- **Mózg:** ESP32-C3 Super Mini (mały, tani, z BLE).
- **Zasilanie logiki:** przetwornica step-down (np. Mini-360) z 2S/3S LiPo do stabilnych 5V dla MCU.
- **Element wykonawczy:** N-MOSFET IRLR7843 (TO-252) sterowany przez gate driver TC4420, żeby uzyskać szybkie i pełne otwarcie bramki przy logice 3.3V.
- **Zabezpieczenia:** dioda Schottky'ego SS54/SS56 na terminalach silników do tłumienia przepięć indukcyjnych.
- **Połączenia:** XT30 i solidne ścieżki z odniesieniem do `/hardware/wiring_diagram.png` (wpięcie oryginalnego Rev Triggera jako wejście logiczne ESP32).

Struktura folderu sprzętowego:
- `/hardware/gerbers` – paczki produkcyjne PCB (ZIP do JLCPCB).
- `/hardware/schematics` – zrzuty schematów z EasyEDA.
- `/hardware/wiring_diagram.png` – diagram połączeń całej instalacji.

## 3. Firmware (Oprogramowanie układowe mikrokontrolera)
Wymagane środowisko: **Visual Studio Code + PlatformIO**.

Architektura kodu firmware:
- `firmware/src/main.cpp` – spina moduły, inicjuje piny i przerwania.
- `firmware/src/ble_server.cpp` + `firmware/src/ble_server.h` – serwer GATT, UUID i obsługa parametrów.
- `firmware/src/pwm_control.cpp` + `firmware/src/pwm_control.h` – wykorzystanie LEDC do cichego PWM i profili spin-up.
- `firmware/include/config.h` – twardo zdefiniowane parametry (np. piny, częstotliwość).
- `firmware/platformio.ini` – konfiguracja środowiska i bibliotek.

## 4. Web Config (Interfejs sterowania i konteneryzacja)
Interfejs to statyczna strona HTML/JS korzystająca z `navigator.bluetooth` — bez frameworków i bez `npm install` dla samego frontu.

- Kod strony: `web-config/src/index.html`, `web-config/src/style.css`, `web-config/src/app.js`.
- Konteneryzacja: `web-config/Dockerfile` oparty o `nginx:alpine`.
- Automatyzacja: `.github/workflows/docker-publish.yml` buduje i publikuje obraz na GHCR przy zmianach w `web-config/`.

Przykładowy flow wdrożenia:
1. Push zmian webowych do repo.
2. GitHub Actions buduje i publikuje nowy obraz.
3. Watchtower (lub inny agent) na serwerze odświeża kontener.
4. Nowa wersja jest dostępna np. pod `nerf.radzu.net`.

## 5. Znane problemy / To-Do
- iOS/Safari nie wspiera natywnie Web Bluetooth. Użytkownicy Apple powinni używać aplikacji **Bluefy**.
- Android + Chrome działa natywnie bez dodatkowych obejść.
