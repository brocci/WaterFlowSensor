/**
 * @file Basic.ino
 * @brief Basic example for YF-B6 water flow sensor
 *
 * Wiring:
 *   Red   -> 5V
 *   Black -> GND
 *   Yellow -> D2 (configurable)
 */

#include <WaterFlowSensor.h>

// YF-B6 sensor on pin D2, 396 pulses/L default, no debounce
WaterFlowSensor sensor(2);

void setup() {
  sensor.begin();
  Serial.begin(115200);
  Serial.println("WaterFlowSensor Test");
}

void loop() {
  if (!sensor.isFlowing()) {
    Serial.println("No flow detected");
    delay(1000);
    return;
  }

  Serial.print("Flow Rate: ");
  Serial.print(sensor.getFlowRate());
  Serial.println(" L/min");

  Serial.print("Total Volume: ");
  Serial.print(sensor.getVolume());
  Serial.println(" mL");

  delay(1000);
}