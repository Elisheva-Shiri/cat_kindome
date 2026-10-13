/*
  ESP32 + MKS TMC2209 V2.0 + 4-wire NEMA17 stepper bring-up test
  Motion: Slow, gentle 30° forward, 1-second pause, 30° back, 1-second pause.

  Wiring matching wiring_esp32_tmc2209.svg (Standard StepStick DIP-16 Layout):
    ESP32 GPIO 25 -> TMC2209 Pin 1  (EN / ENN, active LOW)
    ESP32 GPIO 26 -> TMC2209 Pin 8  (DIR)
    ESP32 GPIO 27 -> TMC2209 Pin 7  (STEP)
    ESP32 3V3     -> TMC2209 Pin 10 (VIO / VDD logic supply)
    ESP32 GND     -> TMC2209 Pin 9  (GND logic) + PSU negative terminal
    12V/24V (+)   -> TMC2209 Pin 16 (VM motor power) + ≥100 µF capacitor (+)
    12V/24V (-)   -> TMC2209 Pin 15 (GND motor power) + ≥100 µF capacitor (-)
    Motor Coil A  -> TMC2209 Pins 12 & 11 (1A / 1B)
    Motor Coil B  -> TMC2209 Pins 13 & 14 (2A / 2B)

  Microstepping:
    Standalone mode: Tie MS1 (Pin 2) & MS2 (Pin 3) to 3.3V for 16 microsteps.
    (If MS1/MS2 are left floating/unconnected, TMC2209 defaults to 8 microsteps).

  Safety:
    - Do NOT power the motor from the ESP32 3V3 or 5V rail.
    - Add a ≥100 µF electrolytic capacitor across VM and GND near the driver.
    - NEVER connect or disconnect the motor while VM is powered.
    - Adjust driver current (Vref trimmer) before running under mechanical load.
*/

#include <Arduino.h>
#include <math.h>

// ---------- Pin Assignments ----------
const int EN_PIN   = 25;   // TMC2209 Pin 1 (EN / ENN), active LOW
const int DIR_PIN  = 26;   // TMC2209 Pin 8 (DIR)
const int STEP_PIN = 27;   // TMC2209 Pin 7 (STEP)

// ---------- Motion Test Settings ----------
const int      FULL_STEPS_PER_REV = 200;    // typical 1.8° stepper motor
const int      MICROSTEPS         = 16;     // set to your MS1/MS2 jumper setting (16 if tied to 3.3V, 8 if floating)
const float    MOVE_DEGREES       = 30.0;   // gentle test movement angle (30 degrees)
const uint32_t ONE_WAY_MS         = 2000;   // move duration: 2 seconds out, 2 seconds back
const uint32_t PAUSE_MS           = 1000;   // clear 1-second pause at each end

// Pulse width for TMC2209 STEP pin
const uint16_t STEP_HIGH_US = 5;
const uint16_t STEP_LOW_US  = 5;

// Total microsteps per test stroke
const long MOVE_STEPS = lround((FULL_STEPS_PER_REV * MICROSTEPS * MOVE_DEGREES) / 360.0);

void stepOnce() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_HIGH_US);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_LOW_US);
}

// Smooth sine S-curve (eases in and out so movement is gentle and silent)
float easeInOutSine(float x) {
  return 0.5f - 0.5f * cosf(PI * x);
}

void moveDegreesSmooth(bool direction) {
  digitalWrite(DIR_PIN, direction ? HIGH : LOW);
  delayMicroseconds(20);  // direction setup time

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
    delay(1); // yield to ESP32 background watchdog
  }

  // Ensure exact step target is reached
  while (stepsAlreadySent < MOVE_STEPS) {
    stepOnce();
    stepsAlreadySent++;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  // Enable the driver (active LOW)
  digitalWrite(EN_PIN, LOW);

  Serial.println("\n==============================================");
  Serial.println("  ESP32 + TMC2209 Stepper Bring-Up Test");
  Serial.println("==============================================");
  Serial.printf("Target move:    %.1f degrees\n", MOVE_DEGREES);
  Serial.printf("Step count:     %ld microsteps each stroke\n", MOVE_STEPS);
  Serial.printf("Duration:       %u ms move, %u ms pause\n", ONE_WAY_MS, PAUSE_MS);
  Serial.println("Driver EN:      LOW (holding torque active)");
  Serial.println("Tip: Try turning the shaft by hand - it should resist.");
  Serial.println("Starting test loop in 2 seconds...\n");
  delay(2000);
}

void loop() {
  Serial.println(">>> Moving forward 30°...");
  moveDegreesSmooth(true);

  Serial.println("--- Pausing at +30° (1 sec)...");
  delay(PAUSE_MS);

  Serial.println("<<< Returning back to 0°...");
  moveDegreesSmooth(false);

  Serial.println("--- Pausing at 0° (1 sec)...\n");
  delay(PAUSE_MS);
}
