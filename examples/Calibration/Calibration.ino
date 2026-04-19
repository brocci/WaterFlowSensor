/**
 * @file Calibration.ino
 * @brief Example demonstrating sensor calibration with storage
 */

#include <Arduino.h>
#include <WaterFlowSensor.h>
#include <EEPROM.h>

// YF-B6 sensor on pin D2
WaterFlowSensor sensor(2);

const int CAL_ADDRESS = 0;

// Storage callback functions using EEPROM
bool eepromRead(int addr, void* data, size_t len) {
  EEPROM.get(addr, data);
  return true;
}

bool eepromWrite(int addr, const void* data, size_t len) {
  const uint8_t* bytes = (const uint8_t*)data;
  for (size_t i = 0; i < len; i++) {
    EEPROM.write(addr + i, bytes[i]);
  }
  return true;
}

void setup() {
  sensor.begin();
  sensor.setStorageCallbacks(eepromRead, eepromWrite);

  if (sensor.loadCalibration(CAL_ADDRESS)) {
    Serial.println("Calibration loaded from EEPROM");
    Serial.print("Pulses per liter: ");
    Serial.println(sensor.getPulsesPerLiter());
  } else {
    Serial.println("No calibration found, using default (396)");
  }

  Serial.begin(115200);
  Serial.println("Water Flow - Calibration Example");
}

void loop() {
  if (!sensor.isFlowing()) {
    delay(500);
    return;
  }

  Serial.print("Flow: ");
  Serial.print(sensor.getFlowRate());
  Serial.println(" L/min");

  Serial.print("Volume: ");
  Serial.print(sensor.getVolume());
  Serial.println(" mL");

  delay(500);
}

/*
 * CALIBRATION PROCEDURE:
 *
 * 1. Run this sketch and dispense known volume (e.g., 1000mL)
 * 2. Compare sensor reading with actual:
 *    long measured = sensor.getVolume();    // e.g., 1050
 *    long actual = 1000;                    // real volume
 *
 * 3. Calibrate:
 *    sensor.calibrate(measured, actual);
 *    sensor.saveCalibration(CAL_ADDRESS);
 *
 * 4. On next boot, calibration loads automatically
 */