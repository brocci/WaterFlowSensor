#ifndef WaterFlowSensor_h
#define WaterFlowSensor_h

#include "Arduino.h"

/**
 * @brief Interrupt edge mode for pulse detection
 */
enum EdgeMode {
  WF_FALLING = 2, ///< Trigger on falling edge (default for YF-B6)
  WF_RISING  = 3, ///< Trigger on rising edge
  WF_CHANGE  = 4  ///< Trigger on any edge change
};

/**
 * @brief Water flow sensor library for Arduino
 * @details Supports YF-B6 and compatible Hall-effect flow sensors
 *
 * @note This library uses interrupts for accurate pulse counting.
 *
 * Basic usage:
 * @code
 * WaterFlowSensor sensor(2);  // Pin D2, 396 pulses/L
 * sensor.begin();
 * 
 * void loop() {
 *   if (sensor.isFlowing()) {
 *     Serial.println(sensor.getFlowRate());  // L/min
 *   }
 * }
 * @endcode
 *
 * For pulse counting from an interrupt handler:
 * @code
 * void myInterrupt() {
 *   sensor.pulseCount();
 * }
 * @endcode
 */
class WaterFlowSensor
{
public:
  /**
   * @brief Construct a new WaterFlowSensor sensor
   * @param pin Digital pin (must support interrupts)
   * @param pulsesPerLiter Pulses per liter (default 396 for YF-B6)
   * @param debounceMicroseconds Debounce in microseconds (default 0 = disabled)
   * @param edgeMode Interrupt edge mode (default FALLING_EDGE)
   *
   * Example:
   * @code
   * WaterFlowSensor sensor(2);                      // pin D2, default 396, no debounce, FALLING
   * WaterFlowSensor sensor(2, 396, 0);              // explicit parameters
   * WaterFlowSensor sensor(2, 396, 0, RISING_EDGE); // use RISING edge
   * @endcode
   */
  WaterFlowSensor(uint8_t pin, uint16_t pulsesPerLiter = 396, uint16_t debounceMicroseconds = 0, EdgeMode edgeMode = WF_FALLING);

  /**
   * @brief Destroy WaterFlowSensor sensor
   */
  ~WaterFlowSensor();

  /**
   * @brief Initialize the sensor
   * @note Must be called from setup()
   *
   * Attaches interrupt and configures pin with internal pull-up.
   */
  void begin();

  /**
   * @brief Check if water is currently flowing
   * @return true if flow detected, false otherwise
   * @note Automatically updates internal state
   */
  bool isFlowing();

  /**
   * @brief Get current flow rate
   * @return Flow rate in liters per minute
   * @note Automatically updates internal state
   */
  float getFlowRate();

  /**
   * @brief Get accumulated volume
   * @return Volume in milliliters
   * @note Automatically updates internal state
   */
  long getVolume();

  /**
   * @brief Reset accumulated volume to zero
   */
  void clearVolume();

  /**
   * @brief Calibrate sensor from measured vs actual volume
   * @param measured Volume shown by sensor (mL)
   * @param actual True volume measured with container/scale (mL)
   *
   * Dispense a known volume (e.g., 1000mL) and compare:
   * - measured = what sensor reported (e.g., 1050mL)
   * - actual = what you actually dispensed (e.g., 1000mL)
   *
   * @note Use when sensor reading is consistently high or low
   *
   * Example:
   * @code
   * // Sensor shows 1050mL, but you dispensed 1000mL
   * sensor.calibrate(1050, 1000);  // reduces pulses/L by ~5%
   * @endcode
   */
  void calibrate(long measured, long actual);

  /**
   * @brief Reset calibration to default value (396 pulses/L)
   */
  void resetCalibration();

  /**
   * @brief Get current pulses per liter value
   * @return Pulses per liter (default 396)
   */
  uint16_t getPulsesPerLiter();

  /**
   * @brief Set storage callbacks for calibration persistence
   * @param readFn Read function: bool(int addr, void* data, size_t len)
   * @param writeFn Write function: bool(int addr, const void* data, size_t len)
   *
   * Allows custom storage (EEPROM, Flash, SD card, etc.)
   *
   * Example with EEPROM:
   * @code
   * sensor.setStorageCallbacks(
   *   [](int addr, void* data, size_t len) { EEPROM.get(addr, data); return true; },
   *   [](int addr, const void* data, size_t len) { EEPROM.put(addr, data); return true; }
   * );
   * @endcode
   */
  void setStorageCallbacks(
    bool (*readFn)(int addr, void* data, size_t len),
    bool (*writeFn)(int addr, const void* data, size_t len)
  );

  /**
   * @brief Save calibration to storage
   * @param address Storage address (default 0)
   * @return true if successful (requires storage callbacks)
   */
  bool saveCalibration(int address = 0);

  /**
   * @brief Load calibration from storage
   * @param address Storage address (default 0)
   * @return true if valid calibration found
   */
  bool loadCalibration(int address = 0);

  /**
   * @brief Set callback for flow start event
   * @param cb Callback function
   *
   * Called when flow is detected after being idle.
   * Called from interrupt context - keep simple!
   */
  void onFlowStart(void (*cb)(void));

  /**
   * @brief Set callback for flow stop event
   * @param cb Callback function
   *
   * Called when flow stops after being detected.
   * Called from interrupt context - keep simple!
   */
  void onFlowStop(void (*cb)(void));

  /**
   * @brief Increment pulse counter
   * @note Call this from your interrupt handler
   *
   * Example:
   * @code
   * void myInterrupt() {
   *   sensor.pulseCount();
   * }
   * @endcode
   */
  void pulseCount();

private:
  void read();

  // Configuration
  uint8_t _pin;
  uint16_t _pulsesPerLiter;
  uint16_t _debounceMicroseconds;
  EdgeMode _edgeMode;

  // Sensor state (accessed from ISR)
  volatile unsigned long _pulse;
  volatile bool _flowing;

  // Cached results (updated by read())
  bool _lastFlowing;
  float _cachedFlowRate;
  long _cachedVolume;

  // Timing
  unsigned long _lastReadTime;
  unsigned long _lastInterruptTime;
  bool _wasFlowing;

  // Callbacks
  void (*_onStart)(void);
  void (*_onStop)(void);

  // Storage
  bool (*_storageRead)(int addr, void* data, size_t len);
  bool (*_storageWrite)(int addr, const void* data, size_t len);
};

#endif