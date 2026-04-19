/**
 * @file Debounce.ino
 * @brief Example with debounce enabled
 */

#include <WaterFlowSensor.h>

// YF-B6 sensor with 1000 microseconds debounce
WaterFlowSensor sensor(2, 396, 1000);

void setup() {
  sensor.begin();
  Serial.begin(115200);
  Serial.println("Water Flow - Debounce Example (1000 us)");
}

void loop() {
  if (!sensor.isFlowing()) {
    Serial.println("No flow");
    delay(500);
    return;
  }

  Serial.print("Flow: ");
  Serial.print(sensor.getFlowRate());
  Serial.println(" L/min");

  delay(500);
}