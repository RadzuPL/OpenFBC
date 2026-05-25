# OpenNerfESC – Coding Guidelines

> This file defines coding conventions for the entire OpenNerfESC project.
> All contributors (human and AI) must follow these rules.

---

## 1. General Rules

- All **variable names, function names, constants and comments inside code** must be in **English**.
- All **user-facing documentation** (README, commit messages) is written in **Polish**.
- Commit messages follow the pattern: `type: short description` (e.g. `feat: add BLE server`, `fix: correct PWM range`).
- No magic numbers – always use named constants defined in `config.h`.
- Keep files short and focused. One responsibility per file.

---

## 2. Firmware (C++ / PlatformIO / Arduino framework)

### File Structure
```
firmware/
  src/
    main.cpp          # Entry point only – setup() and loop()
    ble_server.cpp    # BLE GATT server, characteristic callbacks
    pwm_control.cpp   # PWM generation and spin-up profile logic
  include/
    config.h          # All constants, pin definitions, default values
    ble_server.h
    pwm_control.h
  platformio.ini
```

### Naming Conventions
| Element | Convention | Example |
|---|---|---|
| Variables | camelCase | `spinUpTime`, `targetSpeed` |
| Constants | UPPER_SNAKE_CASE | `DEFAULT_TARGET_SPEED` |
| Functions | camelCase | `initBleServer()` |
| Classes | PascalCase | `BleServer` |
| Files | snake_case | `ble_server.cpp` |

### Debug Mode
- All debug output is wrapped in `#if DEBUG_MODE` / `#endif` blocks.
- `DEBUG_MODE` is defined in `platformio.ini` via `build_flags = -D DEBUG_MODE=1` (or `=0`).
- Never use `Serial.print` outside of `#if DEBUG_MODE` blocks.
- Debug messages should be prefixed with a module tag, e.g. `[BLE]`, `[PWM]`, `[MAIN]`.

### Types
- `spinUpTime` → `uint16_t` (milliseconds, range 0–5000)
- `targetSpeed` → `uint8_t` (percent, range 0–100)
- `minVoltage` → `float` (volts, e.g. 6.0)
- Use explicit types – avoid `int` and `auto` in embedded code.

### Safety Rules
- Always validate incoming BLE values before applying them.
- Never apply out-of-range values silently – log a warning and clamp or reject.
- PWM output must default to 0 on startup and on BLE disconnect.

---

## 3. Web Config (HTML / CSS / JavaScript)

- Plain HTML5 + vanilla JavaScript. No frameworks, no npm.
- All JS variable names in camelCase, matching firmware variable names exactly.
- BLE characteristic UUIDs must be defined as constants at the top of `app.js`.
- The page must handle BLE disconnect gracefully (show status, allow reconnect).
- Input validation on the frontend mirrors firmware validation (same min/max ranges).

---

## 4. Docker / CI

- `Dockerfile` uses `nginx:alpine` as base – do not change the base image without discussion.
- GitHub Actions workflow triggers only on changes to `web-config/**`.
- Image is always tagged as `latest` on push to `main`.

---

## 5. For AI Assistants (Claude, Copilot)

- Follow all rules in this file strictly.
- When generating firmware code, always include `#if DEBUG_MODE` guards around Serial output.
- When generating web code, always define UUIDs as named constants.
- Do not introduce external libraries without explicit approval.
- Variable names in firmware and web must always match exactly.
