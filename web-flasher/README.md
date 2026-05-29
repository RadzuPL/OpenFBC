# Web Flasher — OpenFBC

> **Język / Language:** [🇵🇱 Polski](#-opis) · [🇬🇧 English](#-description)

---

## 🇵🇱 Opis

Web Flasher umożliwia wgranie firmware na ESP32 **bezpośrednio z przeglądarki** — bez instalowania żadnych narzędzi, sterowników ani oprogramowania. Wystarczy kabel USB i Chrome lub Edge. Publiczna instancja dostępna pod adresem **[flasher.radzu.net](https://flasher.radzu.net)**.

> **Wymagania:** Chrome lub Edge (WebSerial API). Firefox i Safari nie są wspierane.

### Jak wgrać firmware?

1. Podłącz ESP32 kablem USB do komputera.
2. Wejdź na **[flasher.radzu.net](https://flasher.radzu.net)** w Chrome lub Edge.
3. Kliknij przycisk odpowiadający Twojej płytce.
4. Przeglądarka pobiera najnowszy firmware z GitHub Releases i wgrywa automatycznie.

### Obsługiwane płytki

| Płytka | Status |
|---|---|
| ESP32-C3 Super Mini | ✅ Docelowy hardware |
| ESP32-C6 Super Mini | 🧪 Platforma testowa |

### Publiczne wdrożenie

Flasher jest dostępny publicznie — **nie musisz nic stawiać samodzielnie**.

### Samodzielne wdrożenie (Docker)

```yaml
services:
  openfbc-flasher:
    image: ghcr.io/radzupl/openfbc-flasher:latest
    container_name: openfbc-flasher
    restart: unless-stopped
    ports:
      - "8322:8080"
```

Obraz jest automatycznie budowany i publikowany na GHCR. Firmware `.bin` jest dołączany do GitHub Releases przy każdym nowym Release.

### Technologia

Flasher oparty jest na bibliotece [ESP Web Tools](https://esphome.github.io/esp-web-tools/) — tej samej co ESPHome i Tasmota.

### Struktura kodu

| Plik | Opis |
|---|---|
| `src/index.html` | Strona flashera |
| `manifests/manifest-esp32c3.json` | Manifest ESP Web Tools dla ESP32-C3 |
| `manifests/manifest-esp32c6.json` | Manifest ESP Web Tools dla ESP32-C6 |
| `Dockerfile` | Obraz nginx:alpine |

---

## 🇬🇧 Description

The Web Flasher allows you to flash firmware to the ESP32 **directly from your browser** — no tools, drivers or software installation needed. All you need is a USB cable and Chrome or Edge. Public instance available at **[flasher.radzu.net](https://flasher.radzu.net)**.

> **Requirements:** Chrome or Edge (WebSerial API). Firefox and Safari are not supported.

### How to Flash Firmware?

1. Connect your ESP32 via USB cable to your computer.
2. Open **[flasher.radzu.net](https://flasher.radzu.net)** in Chrome or Edge.
3. Click the button for your board.
4. The browser downloads the latest firmware from GitHub Releases and flashes automatically.

### Supported Boards

| Board | Status |
|---|---|
| ESP32-C3 Super Mini | ✅ Target hardware |
| ESP32-C6 Super Mini | 🧪 Test platform |

### Public Deployment

The flasher is publicly available — **you don't need to host anything yourself**.

### Self-Hosting (Docker)

```yaml
services:
  openfbc-flasher:
    image: ghcr.io/radzupl/openfbc-flasher:latest
    container_name: openfbc-flasher
    restart: unless-stopped
    ports:
      - "8322:8080"
```

The image is automatically built and published to GHCR. Firmware `.bin` files are attached to GitHub Releases on every new Release.

### Technology

Built on [ESP Web Tools](https://esphome.github.io/esp-web-tools/) — the same library used by ESPHome and Tasmota.

### Code Structure

| File | Description |
|---|---|
| `src/index.html` | Flasher page |
| `manifests/manifest-esp32c3.json` | ESP Web Tools manifest for ESP32-C3 |
| `manifests/manifest-esp32c6.json` | ESP Web Tools manifest for ESP32-C6 |
| `Dockerfile` | nginx:alpine image |
