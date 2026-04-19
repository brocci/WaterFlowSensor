/**
 * @file WithCallbacks.ino
 * @brief Example demonstrating flow start/stop callbacks
 */

#include <WaterFlowSensor.h>

// YF-B6 sensor on pin D2
WaterFlowSensor sensor(2);

void onFlowStart() {
  Serial.println(">> FLOW STARTED <<");
}

void onFlowStop() {
  Serial.println(">> FLOW STOPPED <<");
  Serial.print("Volume this session: ");
  Serial.print(sensor.getVolume());
  Serial.println(" mL");
}

void setup() {
  sensor.begin();
  sensor.onFlowStart(onFlowStart);
  sensor.onFlowStop(onFlowStop);

  Serial.begin(115200);
  Serial.println("Water Flow - Callback Example");
}

void loop() {
  if (sensor.isFlowing()) {
    Serial.print("Flow: ");
    Serial.print(sensor.getFlowRate());
    Serial.println(" L/min");
  }

  delay(500);
}