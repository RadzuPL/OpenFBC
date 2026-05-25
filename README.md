# OpenNerfESC

## ⚠️ UWAGA
**TO DOPIERO SAM POCZĄTEK PROJEKTU — WIĘKSZOŚĆ TEGO, CO JEST TUTAJ AKTUALNIE STWORZONE, PRZYGOTOWAŁ COPILOT.**

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
    

## 5. Znane problemy
- iOS/Safari nie wspiera natywnie Web Bluetooth. Użytkownicy Apple powinni używać aplikacji **Bluefy**.
- Android + Chrome działa natywnie bez dodatkowych obejść.

## 6. TODO (z podziałem na obszary projektu)

### Hardware / elektronika
- [ ] Dokończyć i zweryfikować pełny schemat elektryczny.
- [ ] Narysować finalne PCB pod docelowe elementy (w tym elementy z modułu MP2315).
- [ ] Wykonać ERC/DRC i przegląd obciążalności ścieżek mocy.
- [ ] Zamówić prototypowe płytki PCB.
- [ ] Zlutować i uruchomić pierwsze prototypy.
- [ ] Zweryfikować termikę i stabilność sekcji mocy pod obciążeniem.

### Firmware (ESP32-C3)
- [ ] Dokończyć implementację i konfigurację wszystkich trybów sterowania PWM.
- [ ] Sprawdzić i dostroić parametry bezpieczeństwa (limity, stany awaryjne).
- [ ] Przetestować komunikację BLE i kompatybilność z różnymi telefonami.
- [ ] Uzupełnić proces flashowania i aktualizacji firmware.

### Web Config / aplikacja webowa
- [ ] Dopracować UI i walidację parametrów.
- [ ] Sprawdzić stabilność połączenia Web Bluetooth i obsługę błędów.
- [ ] Uzupełnić dokumentację użytkownika dla konfiguratora.

### Konteneryzacja i deployment
- [x] Skonfigurować kontener Docker dla web-config (`nginx:alpine`).
- [x] Zweryfikować automatyczny build/publish obrazu w GitHub Actions (tag `latest` na branchu `main`).
- [x] Obraz publiczny dostępny na GHCR: `ghcr.io/radzupl/opennerfesc-web-config:latest`.
- [x] Strona placeholder działa i jest dostępna publicznie pod `one.radzu.net`.
- [x] Dopiąć subdomenę do kontenera i potwierdzić dostępność po HTTPS (SSL przez Cloudflare).

### Organizacja projektu i dokumentacja
- [ ] Uporządkować backlog i kolejność prac (MVP → kolejne iteracje).
- [ ] Uzupełnić README o status postępu po każdym większym etapie.
