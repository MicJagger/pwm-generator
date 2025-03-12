#include "analog-output-emulator.hpp"

void AnalogOutputEmulator::SetVoltage(uint8_t pin, uint8_t value) {
  insertionQueue.emplace(pin, value);
}

void AnalogOutputEmulator::Run() {
  // insert new values
  if (!insertionQueue.empty()) {
    mtx.lock();
    for (auto iter = insertionQueue.begin(); iter != insertionQueue.end(); iter++) {
      uint8_t pin = iter->first;
      uint8_t value = iter->second;
      if (analogPins.find(pin) != analogPins.end()) {
        if (value == 0) {
          analogPins.erase(iter->first);
        }
        analogPins[pin] = value;
      }
      else if (value > 0) {
        analogPins.emplace(pin, value);
      }
    }
    insertionQueue.clear();
    mtx.unlock();
  }

  // run i-- pulse loop
  for (int i = 254; i >= 0; i--) {
  //for (int i = 0; i < 255; i++) {
    for (auto iter = analogPins.begin(); iter != analogPins.end(); iter++) {
      uint8_t pin = iter->first;
      uint8_t percentage = iter->second;
      if (percentage > i) {
        digitalWrite(pin, HIGH);
      }
      else {
        digitalWrite(pin, LOW);
      }
    }

    // ensures long enough pulse for stability at all ranges
    delayMicroseconds(1);
  }
}
