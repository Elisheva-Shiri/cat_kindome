# ESP32 + MKS TMC2209 V2.0 one-motor slow swing

This sketch moves one 4-wire stepper motor 20 degrees forward in 5 seconds, then 20 degrees back in 5 seconds, repeatedly.

## Files

- `tmc2209_esp32_stepper.ino` — Arduino/ESP32 sketch.
- `wiring_esp32_tmc2209.svg` — wiring diagram image.

## Wiring summary

| ESP32 / Power Supply | MKS TMC2209 Pin | Header Location | Notes |
|---|---|---|---|
| **ESP32 3V3** | **Pin 10 (VIO / VDD)** | Right Header | 3.3 V logic supply (NOT on left header!) |
| **ESP32 GND** | **Pin 9 (GND Logic)** | Right Header | Common logic ground |
| **GPIO25** | **Pin 1 (EN)** | Left Header | Active LOW enable |
| **GPIO26** | **Pin 8 (DIR)** | Left Header | Direction signal |
| **GPIO27** | **Pin 7 (STEP)** | Left Header | Step pulse signal |
| **12 V / 24 V Supply (+)** | **Pin 16 (VM)** | Right Header | Motor power (add ≥100 µF cap to GND) |
| **Supply (-)** | **Pin 15 (GND Motor)** | Right Header | Motor power ground |
| **Stepper Coil Pair A** | **Pins 12 & 11 (1A / 1B)** | Right Header | Coil A |
| **Stepper Coil Pair B** | **Pins 13 & 14 (2A / 2B)** | Right Header | Coil B |

> **Important Pinout Note:**
> On standard StepStick modules (including MKS TMC2209 V2.0), the **left header (Pins 1–8)** is strictly control/config signals (`EN`, `MS1`, `MS2`, `UART`, `CLK`, `NC`, `STEP`, `DIR`). The **right header (Pins 9–16)** carries all power and motor outputs (`GND`, `VIO`, `1B`, `1A`, `2A`, `2B`, `GND`, `VM`).
>
> Common ground between ESP32 GND and motor supply (-) is required. Do **not** power the motor from the ESP32.

## Microsteps

The sketch assumes `MICROSTEPS = 16`. Wire/set MS1 and MS2 for 16 microsteps, or change `MICROSTEPS` in the sketch to match your driver jumpers.

## Safety

- Add a 100 µF or larger electrolytic capacitor across VM and GND close to the driver.
- Never connect or disconnect the motor while motor power is on.
- Set the driver current/Vref before running under load.
- If the motor only vibrates, one coil pair is wired wrong. Find the two coil pairs with a multimeter.

## Upload

1. Install ESP32 board support in Arduino IDE.
2. Open `tmc2209_esp32_stepper.ino`.
3. Select your ESP32 board and port.
4. Upload.
