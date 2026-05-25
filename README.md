# OpenNerfESC

## ⚠️ UWAGA
**TO DOPIERO SAM POCZĄTEK PROJEKTU — WIĘKSZOŚĆ TEGO, CO JEST TUTAJ AKTUALNIE STWORZONE, PRZYGOTOWAŁ COPILOT.**

## 1. Wstęp i założenia projektu
OpenNerfESC to otwartoźródłowy, miniaturowy i bezdźwiękowy (20kHz+) sterownik PWM do silników szczotkowych Flywheel w wyrzutniach strzałkowych. Sterowanie odbywa się z poziomu przeglądarki przez Web Bluetooth API, więc nie trzeba rozkręcać wyrzutni i kręcić potencjometrem.

## 2. Hardware (Elektronika i „Kanapka”)
 - **Mózg:** TENSTAR ESP32-C3 Super Mini – https://pl.aliexpress.com/item/1005009890133886.html
 - **Driver bramki MOSFET:** TC4420 – https://pl.aliexpress.com/item/1005011629643514.html
 - **Element wykonawczy:** N-MOSFET IRLR7843 (TO-252) – https://pl.aliexpress.com/item/1005006127790007.html
 - **Zabezpieczenia:** dioda Schottky’ego SB540 5A40V – https://pl.aliexpress.com/item/1005007048222899.html
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
  Faza 2 (Cruise):    PWM = targetSpeed [%]   (aż do zwolnienia spustu)

Trigger zwolniony:
  Natychmiastowy stop: PWM = 0%
```

### Parametry konfiguracyjne (ustawiane przez BLE / Web Config)

| Parametr | Zakres | Domyślnie | Opis |
|---|---|---|---|
| `spinUpTime` | 0 – 500 ms | 200 ms | Czas trwania fazy Spin-Up na pełnej mocy (100% PWM). Ustawienie 0 pomija fazę Spin-Up i od razu przechodzi do Cruise. |
| `targetSpeed` | 0 – 100 % | 75 % | Wypełnienie PWM w fazie Cruise — prędkość robocza silników podczas włączonego spustu. |
| `minVoltage` | 3.0 – 15.0 V | 11.1 V | Minimalne napięcie pakietu LiPo. Poniżej tej granicy wyrzutnia jest blokowana (ochrona akumulatora). |

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
| Trigger (spust) | GPIO 4 | Wejście logiczne, active HIGH |
| ADC napięcia | GPIO 1 | Dzielnik napięcia LiPo |

## 5. Web Config (Interfejs sterowania i konteneryzacja)
Jest to statyczna strona HTML/JS korzystająca z `navigator.bluetooth` – bez frameworków i bez `npm install` dla samego frontu.

 - Kod strony: `web-config/src/index.html`, `web-config/src/style.css`, `web-config/src/app.js`.
 - Konteneryzacja: `web-config/Dockerfile` oparty o `nginx:alpine`.
 - Automatyzacja: `.github/workflows/docker-publish.yml` buduje i publikuje obraz na GHCR przy zmianach w `web-config/`.
 - Publiczny interfejs dostepny pod adresem: **https://one.radzu.net**

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

## 6. Znane problemy
 - iOS/Safari nie wspiera natywnie Web Bluetooth. Użytkownicy Apple powinni używać aplikacji **Bluefy**.

## 7. TODO (z podziałem na obszary projektu)
 - [ ] Hardware: zaprojektować PCB w EasyEDA z IRLR7843 + TC4420 + SB540 + zasilaczem 5V.
 - [ ] Hardware: zamawiłem IRLR7843 (TO-252) i czekam na dostawę.
 - [ ] Firmware: implementacja odczytu ADC napięcia i blokady przy `minVoltage`.
 - [ ] Firmware: testy na docelowym ESP32-C3 Super Mini (aktualnie testy na ESP32-C6 DevKitC-1).
 - [ ] Web Config: dodanie pola minVoltage do UI.
 - [ ] Testy: weryfikacja działania Spin-Up/Cruise na fizycznym silniku.
