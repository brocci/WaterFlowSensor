#include "Arduino.h"
#include "WaterFlowSensor.h"

#ifdef ESP8266
#define WF_IRAM_ATTR IRAM_ATTR
#else
#define WF_IRAM_ATTR
#endif

#define WF_CRITICAL_ENTER()   noInterrupts()
#define WF_CRITICAL_EXIT()    interrupts()

static WaterFlowSensor* _instance = nullptr;

static void WF_IRAM_ATTR _pulseHandler() {
  if (_instance) {
    _instance->pulseCount();
  }
}

static const uint16_t CAL_MAGIC = 0xCA1B;

// --------------------------------
// Constructor
// --------------------------------
WaterFlowSensor::WaterFlowSensor(uint8_t pin, uint16_t pulsesPerLiter, uint16_t debounceMicroseconds, EdgeMode edgeMode)
  : _pin(pin)
  , _pulsesPerLiter(pulsesPerLiter)
  , _debounceMicroseconds(debounceMicroseconds)
  , _edgeMode(edgeMode)
  , _pulse(0)
  , _flowing(false)
  , _lastFlowing(false)
  , _cachedFlowRate(0.0f)
  , _cachedVolume(0)
  , _lastReadTime(0)
  , _lastInterruptTime(0)
  , _wasFlowing(false)
  , _onStart(nullptr)
  , _onStop(nullptr)
  , _storageRead(nullptr)
  , _storageWrite(nullptr)
{
}

WaterFlowSensor::~WaterFlowSensor() {
}

// --------------------
// Initialization
// --------------------
void WaterFlowSensor::begin() {
  _instance = this;
  pinMode(_pin, INPUT);
  digitalWrite(_pin, INPUT_PULLUP);
  int mode = LOW;
  switch (_edgeMode) {
    case WF_FALLING: mode = FALLING; break;
    case WF_RISING:  mode = RISING;  break;
    case WF_CHANGE:  mode = CHANGE;  break;
  }
  attachInterrupt(digitalPinToInterrupt(_pin), _pulseHandler, mode);
}

// --------------------
// ISR Handler
// --------------------
void WaterFlowSensor::pulseCount() {
  unsigned long now = micros();

  if (_debounceMicroseconds > 0) {
    if ((now - _lastInterruptTime) < _debounceMicroseconds) {
      return;
    }
    _lastInterruptTime = now;
  }

  bool wasNotFlowing = !_flowing;

  WF_CRITICAL_ENTER();
  _pulse++;
  _flowing = true;
  WF_CRITICAL_EXIT();

  if (wasNotFlowing && _onStart) {
    _onStart();
  }
}

// --------------------
// Internal Read
// --------------------
void WaterFlowSensor::read() {
  unsigned long now = millis();
  if ((now - _lastReadTime) < 1000) {
    return;
  }

  // Single critical section to capture sensor state
  unsigned long pulsesNow;
  bool currentFlowing;

  WF_CRITICAL_ENTER();
  pulsesNow = _pulse;
  currentFlowing = _flowing;
  _pulse = 0;
  _flowing = false;
  WF_CRITICAL_EXIT();

  // Handle flow stop event
  if (_wasFlowing && !currentFlowing && _onStop) {
    _onStop();
  }

  _wasFlowing = currentFlowing;

  if (pulsesNow == 0) {
    _lastFlowing = false;
    _cachedFlowRate = 0.0f;
    _lastReadTime = now;
    return;
  }

  float elapsed = (float)(now - _lastReadTime);
  _cachedFlowRate = ((1000.0f / elapsed) * (float)pulsesNow) / (float)_pulsesPerLiter;
  _cachedVolume += (long)((_cachedFlowRate / 60.0f) * 1000.0f);

  _lastFlowing = true;
  _lastReadTime = now;
}

// --------------------
// Data Access
// --------------------
bool WaterFlowSensor::isFlowing() {
  read();
  return _lastFlowing;
}

float WaterFlowSensor::getFlowRate() {
  read();
  return _cachedFlowRate;
}

long WaterFlowSensor::getVolume() {
  read();
  return _cachedVolume;
}

void WaterFlowSensor::clearVolume() {
  _cachedVolume = 0;
}

// --------------------
// Calibration
// --------------------
void WaterFlowSensor::calibrate(long measured, long actual) {
  if (measured > 0) {
    _pulsesPerLiter = (uint16_t)((float)_pulsesPerLiter * (float)actual / (float)measured);
  }
}

void WaterFlowSensor::resetCalibration() {
  _pulsesPerLiter = 396;
}

uint16_t WaterFlowSensor::getPulsesPerLiter() {
  return _pulsesPerLiter;
}

// --------------------
// Storage
// --------------------
void WaterFlowSensor::setStorageCallbacks(
  bool (*readFn)(int addr, void* data, size_t len),
  bool (*writeFn)(int addr, const void* data, size_t len))
{
  _storageRead = readFn;
  _storageWrite = writeFn;
}

bool WaterFlowSensor::saveCalibration(int address) {
  if (!_storageRead || !_storageWrite) {
    return false;
  }

  if (!_storageWrite(address, &CAL_MAGIC, sizeof(CAL_MAGIC))) {
    return false;
  }
  if (!_storageWrite(address + 2, &_pulsesPerLiter, sizeof(_pulsesPerLiter))) {
    return false;
  }
  return true;
}

bool WaterFlowSensor::loadCalibration(int address) {
  if (!_storageRead) {
    return false;
  }

  uint16_t magic;
  if (!_storageRead(address, &magic, sizeof(magic))) {
    return false;
  }
  if (magic != CAL_MAGIC) {
    return false;
  }
  if (!_storageRead(address + 2, &_pulsesPerLiter, sizeof(_pulsesPerLiter))) {
    return false;
  }
  return true;
}

// --------------------
// Events
// --------------------
void WaterFlowSensor::onFlowStart(void (*cb)(void)) {
  _onStart = cb;
}

void WaterFlowSensor::onFlowStop(void (*cb)(void)) {
  _onStop = cb;
}