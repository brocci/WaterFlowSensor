# Changelog

All notable changes to this library will be documented in this file.

## [2.0.1] - 2026-04-19

### Added
- Edge mode parameter in constructor (FALLING_EDGE, RISING_EDGE, CHANGE_EDGE)
- Debounce in constructor
- Storage callbacks using function pointers (no EEPROM dependency)
- `clearVolume()`, `getPulsesPerLiter()`, `calibrate()`, `resetCalibration()`
- Flow callbacks: `onFlowStart()`, `onFlowStop()`

### Changed
- Simplified public API
- Constructor default pulsesPerLiter = 396
- `getFlowRate()` now single method returning L/min
- `begin()` handles ISR internally (removed callback parameter)
- Made `read()` internal (called automatically by getters)
- Single critical section optimization (one entry/exit per cycle)

### Fixed (from 1.2.0)
- Type mismatch: sensor constant changed from `uint8_t` to `uint16_t`
- Race condition: interrupt no longer detached during calculation
- Added critical sections for all shared variable access
- Volatile qualifiers added for thread-safe variables
- Uninitialized function pointers now set to nullptr

## [1.2.0]

- Initial release (official)