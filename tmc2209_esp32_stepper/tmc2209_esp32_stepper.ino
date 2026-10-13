/*
  ESP32 + MKS TMC2209 V2.0 + 4-wire NEMA17 stepper
  Motion: 20 degrees forward in 5 seconds, then 20 degrees back in 5 seconds, repeat.

  Wiring used by this sketch:
    ESP32 GPIO 25 -> TMC2209 EN   (LOW = enabled)
    ESP32 GPIO 26 -> TMC2209 DIR
    ESP32 GPIO 27 -> TMC2209 STEP
    ESP32 3V3     -> TMC2209 VIO
    ESP32 GND     -> TMC2209 GND logic + motor supply GND
    12V/24V +     -> TMC2209 VM
    12V/24V -     -> TMC2209 GND motor
    Stepper coil A -> M1A / M1B
    Stepper coil B -> M2A / M2B

  Microstep pins:
    Recommended for smooth slow motion: MS1 = HIGH, MS2 = HIGH for 16 microsteps
    on the TMC2209 IC. If your board/jumpers use a different microstep mode,
    change MICROSTEPS below to match.

  Safety:
    - Do NOT power the motor from the ESP32.
    - Add >=100 uF electrolytic capacitor across VM and GND near the driver.
    - Never connect/disconnect the motor while VM is powered.
    - Set driver current/Vref before loading the motor.
*/

#include <Arduino.h>
#include <math.h>

// ---------- Pins ----------
const int EN_PIN   = 25;   // TMC2209 EN / ENN, active LOW
const int DIR_PIN  = 26;   // TMC2209 DIR
const int STEP_PIN = 27;   // TMC2209 STEP

// ---------- Motor / movement settings ----------
const int   FULL_STEPS_PER_REV = 200;     // typical 1.8 degree stepper
const int   MICROSTEPS         = 16;      // set to your MS1/MS2 jumper mode
const float MOVE_DEGREES       = 20.0;    // requested angle
const uint32_t ONE_WAY_MS      = 5000;    // 5 sec out, 5 sec back = 10 sec total
const uint32_t PAUSE_MS        = 0;       // keep 0 for exactly 10 sec total; try 250 for a soft end pause

// TMC2209 needs only a short high pulse; keep it conservative for ESP32/Arduino.
const uint16_t STEP_HIGH_US = 5;
const uint16_t STEP_LOW_US  = 5;

const long MOVE_STEPS = lround((FULL_STEPS_PER_REV * MICROSTEPS * MOVE_DEGREES) / 360.0);

void stepOnce() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_HIGH_US);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_LOW_US);
}

// Smooth position curve: starts slowly, speeds up gently, slows at the end.
float easeInOutSine(float x) {
  return 0.5f - 0.5f * cosf(PI * x);
}

void moveDegreesSmooth(bool direction) {
  digitalWrite(DIR_PIN, direction ? HIGH : LOW);
  delayMicroseconds(20);  // direction setup time before first STEP pulse

  uint32_t startMs = millis();
  long stepsAlreadySent = 0;

  while (true) {
    uint32_t elapsed = millis() - startMs;
    if (elapsed > ONE_WAY_MS) elapsed = ONE_WAY_MS;

    float progress = (float)elapsed / (float)ONE_WAY_MS;
    float eased = easeInOutSine(progress);
    long targetSteps = lround((float)MOVE_STEPS * eased);

    while (stepsAlreadySent < targetSteps) {
      stepOnce();
      stepsAlreadySent++;
    }

    if (elapsed >= ONE_WAY_MS) break;
    delay(1); // yield to ESP32 background tasks
  }

  // Ensure rounding did not leave us one step short.
  while (stepsAlreadySent < MOVE_STEPS) {
    stepOnce();
    stepsAlreadySent++;
  }
}

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  // Enable the driver. If you want motor coils off between moves, set EN high after each move.
  digitalWrite(EN_PIN, LOW);

  Serial.begin(115200);
  Serial.println("ESP32 + TMC2209: 20 degrees out/back over 10 seconds, repeating");
  Serial.print("Move steps each way: ");
  Serial.println(MOVE_STEPS);
}

void loop() {
  moveDegreesSmooth(true);   // go 20 degrees
  delay(PAUSE_MS);
  moveDegreesSmooth(false);  // return 20 degrees
  delay(PAUSE_MS);
}

