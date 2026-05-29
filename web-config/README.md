# Web Config — OpenFBC

> **Język / Language:** [🇵🇱 Polski](#-opis) · [🇬🇧 English](#-description)

---

## 🇵🇱 Opis

Web Konfigurator to statyczna strona HTML/JS komunikująca się z kontrolerem OpenFBC przez **Web Bluetooth API (BLE)**. Pozwala konfigurować wszystkie parametry pracy wyrzutni bez instalowania aplikacji. Publiczna instancja dostępna pod adresem **[one.radzu.net](https://one.radzu.net)**.

> **iOS/Safari:** Web Bluetooth nie jest natywnie wspierany. Użyj aplikacji **[Bluefy](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055)**.

### Publiczne wdrożenie

Konfigurator jest dostępny publicznie — **nie musisz nic stawiać samodzielnie**. Wystarczy wejść na [one.radzu.net](https://one.radzu.net) z urządzenia z Bluetooth.

### Samodzielne wdrożenie (Docker)

Jeśli chcesz postawić własną instancję:

```yaml
services:
  openfbc-web:
    image: ghcr.io/radzupl/openfbc-web-config:latest
    container_name: openfbc-web
    restart: unless-stopped
    ports:
      - "8321:80"
```

Obraz jest automatycznie budowany i publikowany na GHCR przy każdym push do `web-config/` na branchu `main` (tagi: `latest`, `main`, SHA commita).

### Struktura kodu

| Plik | Opis |
|---|---|
| `src/index.html` | Główna strona konfiguratora |
| `src/style.css` | Style interfejsu |
| `src/app.js` | Logika BLE i obsługa parametrów |
| `Dockerfile` | Obraz nginx:alpine serwujący statyczne pliki |

---

## 🇬🇧 Description

The Web Configurator is a static HTML/JS page that communicates with the OpenFBC controller via **Web Bluetooth API (BLE)**. It lets you configure all blaster operating parameters without installing any app. Public instance available at **[one.radzu.net](https://one.radzu.net)**.

> **iOS/Safari:** Web Bluetooth is not natively supported. Use the **[Bluefy](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055)** app.

### Public Deployment

The configurator is publicly available — **you don't need to host anything yourself**. Just open [one.radzu.net](https://one.radzu.net) on a device with Bluetooth.

### Self-Hosting (Docker)

If you want to run your own instance:

```yaml
services:
  openfbc-web:
    image: ghcr.io/radzupl/openfbc-web-config:latest
    container_name: openfbc-web
    restart: unless-stopped
    ports:
      - "8321:80"
```

The image is automatically built and published to GHCR on every push to `web-config/` on the `main` branch (tags: `latest`, `main`, commit SHA).

### Code Structure

| File | Description |
|---|---|
| `src/index.html` | Main configurator page |
| `src/style.css` | UI styles |
| `src/app.js` | BLE logic and parameter handling |
| `Dockerfile` | nginx:alpine image serving static files |
