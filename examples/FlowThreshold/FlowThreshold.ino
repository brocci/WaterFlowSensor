/**
 * @file WithFlowThreshold.ino
 * @brief Example demonstrating flow detection with threshold
 */

#include <WaterFlowSensor.h>

// YF-B6 sensor on pin D2
WaterFlowSensor sensor(2);

const float MIN_FLOW_THRESHOLD = 0.5;   // L/min
const long TARGET_VOLUME = 1000;        // mL

bool targetReached = false;

void setup() {
  sensor.begin();

  Serial.begin(115200);
  Serial.println("Water Flow - Threshold Example");
  Serial.print("Target volume: ");
  Serial.print(TARGET_VOLUME);
  Serial.println(" mL");
}

void loop() {
  float flowRate = sensor.getFlowRate();
  long volume = sensor.getVolume();

  if (flowRate >= MIN_FLOW_THRESHOLD) {
    Serial.print("Flowing: ");
    Serial.print(flowRate);
    Serial.println(" L/min");
  }

  Serial.print("Volume: ");
  Serial.print(volume);
  Serial.println(" mL");

  if (!targetReached && volume >= TARGET_VOLUME) {
    targetReached = true;
    Serial.println("*** TARGET REACHED ***");
  }

  delay(500);
}