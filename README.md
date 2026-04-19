## Description

Arduino library for Hall-effect water flow sensors.

### Supported Sensors

- **YF-B6** (1/2" BSP, 396 pulses/L) - Primary supported model
- Other similar water flow sensor generating pulses

### Flowrate Formula

```
Flow Rate (L/min) = (1000ms / elapsed_ms) * pulses / pulsesPerLiter
```

Default pulses per liter: **396**

## Installation

1. Download this library
2. Place in Arduino libraries folder
3. Restart Arduino IDE

## Quick Start

```cpp
#include <WaterFlowSensor.h>

// Pin D2, default 396 pulses/L
WaterFlowSensor sensor(2);

void setup() {
  sensor.begin();
  Serial.begin(115200);
}

void loop() {
  if (sensor.isFlowing()) {
    Serial.print("Flow: ");
    Serial.print(sensor.getFlowRate());
    Serial.println(" L/min");

    Serial.print("Volume: ");
    Serial.print(sensor.getVolume());
    Serial.println(" mL");
  }
  delay(500);
}
```

## Constructor

```cpp
WaterFlowSensor(uint8_t pin, uint16_t pulsesPerLiter = 396, uint16_t debounceMicroseconds = 0, EdgeMode edgeMode = WF_FALLING);
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| pin | - | Digital pin (must support interrupts) |
| pulsesPerLiter | 396 | Sensor pulses per liter |
| debounceMicroseconds | 0 | Debounce filter (0 = disabled) |
| edgeMode | WF_FALLING | Interrupt edge mode |

### Edge Mode Options

```cpp
WF_FALLING  // Trigger on falling edge (default, recommended for YF-B6)
WF_RISING   // Trigger on rising edge
WF_CHANGE   // Trigger on any edge change

// Example with RISING edge
WaterFlowSensor sensor(2, 450, 0, WF_RISING);
```

## API Reference

### Basic Methods

```cpp
void begin();              // Initialize - call from setup()
bool isFlowing();          // Is water flowing?
float getFlowRate();       // Flow rate in L/min
long getVolume();          // Accumulated volume in mL
void clearVolume();        // Reset volume to zero
```

### Calibration

```cpp
void calibrate(long measured, long actual);  // Auto-calibrate
void resetCalibration();                      // Reset to 396
uint16_t getPulsesPerLiter();                 // Get current value
```

**How it works:**

| Parameter | Description |
|-----------|-------------|
| measured | Volume shown by sensor (mL) |
| actual | True volume (measured with container/scale) (mL) |

**Calibration procedure:**

1. Dispense a known volume (e.g., 1000mL) into a container
2. Read sensor.getVolume() → measured (e.g., shows 1050mL)
3. Call calibrate(measured, actual):
   - `sensor.calibrate(1050, 1000);`

The sensor adjusts its pulses-per-liter to match reality.

**Example:**
```cpp
// Sensor measures 1050mL, actual is 1000mL
sensor.calibrate(1050, 1000);

// Now readings will be ~5% lower
```

### Events (Optional)

```cpp
void onFlowStart(void (*cb)(void));  // Called when flow starts
void onFlowStop(void (*cb)(void));   // Called when flow stops
```

## Wiring

```
YF-B6 Sensor:
  Red     -> 5V (or 3.3V)
  Black   -> GND
  Yellow  -> Arduino interrupt pin (e.g., D2)
```

## Storage Example (EEPROM)

```cpp
#include <EEPROM.h>

bool eepromRead(int addr, void* data, size_t len) {
  EEPROM.get(addr, data);
  return true;
}

bool eepromWrite(int addr, const void* data, size_t len) {
  EEPROM.put(addr, data);
  return true;
}

void setup() {
  sensor.begin();
  sensor.setStorageCallbacks(eepromRead, eepromWrite);
  sensor.loadCalibration(0);  // Load from address 0
}

void calibrateSensor(long actualVolume) {
  long measured = sensor.getVolume();
  sensor.calibrate(measured, actualVolume);
  sensor.saveCalibration(0);
}
```

## Remote Configuration (Advanced)

For remote calibration updates (WiFi/API), set the pulses per liter before `begin()`:

```cpp
#include <WiFi.h>

// Sensor with defaults initially
WaterFlowSensor sensor(2);

void setup() {
  // Connect to WiFi
  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // Fetch calibration from HTTP API
  HTTPClient http;
  http.begin("http://your-server.com/calibration.json");
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    int pulsesPerLiter = payload.toInt();
    // Use constructor to set custom pulsesPerLiter before begin()
    // WaterFlowSensor sensor(2, pulsesPerLiter);
  }
  
  // Now begin with remote config
  sensor.begin();
}
```

Or use the calibrate method directly:

```cpp
void setup() {
  // ... fetch measured/actual from API ...
  long measured = 1020;  // from API
  long actual = 1000;    // from API
  
  sensor.begin();
  sensor.calibrate(measured, actual);
}
```

## Troubleshooting

1. **Flow reads 0:** Check wiring, ensure pull-up enabled
2. **Flow too high/low:** Use `calibrate(measured, actual)` to adjust
3. **Intermittent readings:** Try adding debounce in constructor

## License

MIT