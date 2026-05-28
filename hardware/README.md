# OpenFBC PowerBoard v1 — Hardware Documentation

> **Status:** Prototype v1 — PCB files ready for ordering, first batch not yet assembled.

## Overview

PowerBoard v1 is a compact carrier board designed to mount directly under an **ESP32-C3 Super Mini** in a sandwich configuration via 2.54 mm pin headers. The board footprint matches the ESP32-C3 Super Mini outline, keeping the total assembly as small as possible. Components are populated on **both sides** of the PCB to achieve this.

The board provides all power and signal conditioning needed to drive two parallel brushed DC flywheel motors from a LiPo battery pack (3S default, 4S compatible).

---

## Functional Blocks

| Block | Key Component | Description |
|---|---|---|
| **5 V Buck Regulator** | MP2315 | Steps down battery voltage (8–24 V) to 5 V for ESP32 and gate driver supply |
| **Gate Driver** | TC4420 | Drives the N-MOSFET gate with fast charge/discharge for clean PWM switching |
| **Motor Switch** | IRLR7843 (TO-252) | N-channel power MOSFET; switches common GND of both motors via PWM |
| **Battery Voltage Monitor** | R21, R22, C13 | Resistive divider + filter cap feeds battery voltage to ESP32 ADC (GPIO 1) |
| **Trigger Input** | Solder pad | Direct wire-solder pad; connects to original blaster Rev trigger (short to GND = active) |

### Voltage Divider (Battery Monitor)

Divider values chosen for compatibility with ESP32-C3, C6 and S3 (ADC safe range ≤ 2.5 V for C3/C6):

- **R21 = 300 kΩ**, **R22 = 51 kΩ**, **C13 = 100 nF** (charge-bucket across R22)
- Ratio: `(300 + 51) / 51 = 6.882`
- ADC pin voltage at **4S full (16.8 V):** ~2.44 V ✅
- ADC pin voltage at **3S full (12.6 V):** ~1.83 V ✅
- Firmware constant: `BATTERY_DIVIDER_RATIO 6.882f` (see `firmware/include/config.h`)

---

## PCB Images

### 3D Render

![3D PCB view 1](3D_PCB.png)
![3D PCB view 2](3D_PCB_2.png)

### 2D Layout

| Top Side | Bottom Side |
|:---:|:---:|
| ![Top](2D_PCB_Top.png) | ![Bottom](2D_PCB_Bottom.png) |

### Schematic

![Schematic](SCH_Shematic.png)

> ⚠️ Low-resolution placeholder — will be replaced with a higher-resolution export.

### Wiring Diagram

![Wiring diagram](wiring_diagram.png)

---

## Production Files

| File | Description |
|---|---|
| [`Gerber_PCB1_2026-05-28.zip`](Gerber_PCB1_2026-05-28.zip) | Gerber package — ready to upload to JLCPCB / PCBWay / similar |

> Default fab settings (JLCPCB): 2-layer, 1.6 mm FR4, HASL, green soldermask — no special settings required.

---

## Bill of Materials (BOM)

All SMD components are **0805** package unless stated otherwise. LCSC part numbers provided for convenient ordering.

| Ref | Value | Description | Package | Qty | LCSC |
|---|---|---|---|---|---|
| C1, C2 | 22 µF | Bulk capacitor (buck regulator) | C1210 | 2 | [C21397](https://www.lcsc.com/product-detail/C21397.html) |
| C4, C5, C11, C13 | 100 nF | Bypass / ADC charge-bucket cap | C0805 | 4 | [C16780](https://www.lcsc.com/product-detail/C16780.html) |
| C12 | 1 µF | Bypass capacitor | C0805 | 1 | [C28323](https://www.lcsc.com/product-detail/C28323.html) |
| L1 | 4.7 µH | Buck inductor | IND 11.6×10.1 mm | 1 | [C6364675](https://www.lcsc.com/product-detail/C6364675.html) |
| R1 | 40.2 kΩ | Buck regulator feedback network | R0805 | 1 | [C2933438](https://www.lcsc.com/product-detail/C2933438.html) |
| R2 | 7.5 kΩ | Buck regulator feedback network | R0805 | 1 | [C2930310](https://www.lcsc.com/product-detail/C2930310.html) |
| R4 | 75 kΩ | Buck regulator support network | R0805 | 1 | [C17819](https://www.lcsc.com/product-detail/C17819.html) |
| R5 | 20 Ω | Buck regulator support network | R0805 | 1 | [C2907241](https://www.lcsc.com/product-detail/C2907241.html) |
| R6 | 200 kΩ | Buck regulator support network | R0805 | 1 | [C2907238](https://www.lcsc.com/product-detail/C2907238.html) |
| R9 | 20 kΩ | Buck regulator support network | R0805 | 1 | [C4328](https://www.lcsc.com/product-detail/C4328.html) |
| R11 | 10 Ω | Gate series resistor (MOSFET) | R0805 | 1 | [C17415](https://www.lcsc.com/product-detail/C17415.html) |
| R12 | 100 kΩ | Gate pull-down resistor (MOSFET) | R0805 | 1 | [C149504](https://www.lcsc.com/product-detail/C149504.html) |
| R21 | 300 kΩ | Battery voltage divider (top) | R0805 | 1 | [C104213](https://www.lcsc.com/product-detail/C104213.html) |
| R22 | 51 kΩ | Battery voltage divider (bottom) | R0805 | 1 | [C2930300](https://www.lcsc.com/product-detail/C2930300.html) |
| Q1 | IRLR7843 | N-channel power MOSFET | TO-252-2 | 1 | [C21988](https://www.lcsc.com/product-detail/C21988.html) |
| U1 | TC4420 | MOSFET gate driver | SOP-8 | 1 | [C231865](https://www.lcsc.com/product-detail/C231865.html) |
| U2 | MP2315 | 3 A synchronous buck regulator | TSOT23-8 | 1 | *(see note)* |
| MCU1 | ESP32-C3 Super Mini | Microcontroller (not on PCB BOM) | Via pin headers | 1 | — |

> **MP2315 note:** Not available on LCSC; source from AliExpress or local distributor. Search: *MP2315 TSOT23-8*.

> **Pin headers / connectors:** Not included in BOM. 2.54 mm pin headers typically come bundled with the ESP32-C3 Super Mini. All other connections (battery, motors, trigger) are direct solder pads — no connectors by design, to minimise board size.

---

## Assembly Notes

- Solder **bottom-side** SMD components first (reflow or hot air), then top-side.
- The ESP32-C3 Super Mini is **not** soldered directly — it plugs in via 2.54 mm pin headers, allowing replacement without rework.
- **Trigger pad:** Solder a two-wire cable directly to the pad. Shorting this pad to GND activates the trigger (active LOW, GPIO 4 on ESP32, internal pull-up enabled in firmware).
- **Motor pads:** Solder motor wires directly to the board. Both motors are wired in parallel to a single MOSFET drain pad.
- **Battery input:** Solder battery leads directly to the BAT+/BAT− pads. Polarity is marked on silkscreen.
- After assembly, verify 5 V rail with a multimeter before plugging in the ESP32.
