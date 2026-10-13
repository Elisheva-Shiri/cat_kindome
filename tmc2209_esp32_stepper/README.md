# ESP32 + MKS TMC2209 V2.0 one-motor slow swing

This sketch moves one 4-wire stepper motor 20 degrees forward in 5 seconds, then 20 degrees back in 5 seconds, repeatedly.

## Files

- `tmc2209_esp32_stepper.ino` — Arduino/ESP32 sketch.
- `wiring_esp32_tmc2209.svg` — wiring diagram image.

## Wiring summary

| ESP32 / supply | MKS TMC2209 pin |
|---|---|
| ESP32 3V3 | VIO |
| ESP32 GND | GND logic |
| GPIO25 | EN |
| GPIO26 | DIR |
| GPIO27 | STEP |
| 12 V or 24 V supply + | VM |
| supply - | GND motor |
| Stepper coil pair A | M1A / M1B |
| Stepper coil pair B | M2A / M2B |

Also connect ESP32 GND and motor-supply negative together. Do **not** power the motor from the ESP32.

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
