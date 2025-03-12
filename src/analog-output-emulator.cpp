#include "analog-output-emulator.hpp"

AnalogOutputEmulator::AnalogOutputEmulator() {
  _cycleCount = 4;
  _cycleLengthExtension = 8;
  _queued = false;
  _skewHIGH = true;
}

void AnalogOutputEmulator::SetVoltage(uint8_t pin, uint8_t value) {
  _mutex.lock();
  _insertionQueue.emplace(pin, value);
  _queued = true;
  _mutex.unlock();
}

void AnalogOutputEmulator::SetCycleCount(uint16_t cycleCount) {
  _mutex.lock();
  _cycleCount = cycleCount;
  _mutex.unlock();
}

void AnalogOutputEmulator::SetCycleLengthExtension(uint16_t cycleLengthExtension) {
  _mutex.lock();
  _cycleLengthExtension = cycleLengthExtension;
  _mutex.unlock();
}

void AnalogOutputEmulator::SetSkew(bool skewHIGH) {
  _mutex.lock();
  _skewHIGH = skewHIGH;
  _mutex.unlock();
}

void AnalogOutputEmulator::Run() {
  _mutex.lock();

  if (_queued) {
    uint8_t pin;
    uint8_t value;
    for (auto iter = _insertionQueue.begin(); iter != _insertionQueue.end(); iter++) {
      pin = iter->first;
      value = iter->second;
      if (value == 0) {
        _analogPins.erase(pin);
        digitalWrite(pin, 0);
      }
      else if (_analogPins.find(pin) != _analogPins.end()) {
        _analogPins[pin] = value;
      }
      else {
        _analogPins.emplace(pin, value);
      }
    }
    _insertionQueue.clear();
    _queued = false;
  }
  
  _mutex.unlock();

  // skewed up (default, usually closer to accurate)
  if (_skewHIGH) {
    for (int cycle = 0; cycle < _cycleCount; cycle++) {

      // run i-- pulse loop
      for (int i = 254; i >= 0; i--) {
          
        for (auto iter = _analogPins.begin(); iter != _analogPins.end(); iter++) {
          uint8_t pin = iter->first;
          uint8_t value = iter->second;
          if (value > i) {
            digitalWrite(pin, HIGH);
          }
          else {
            digitalWrite(pin, LOW);
          }
        }

        delayMicroseconds(_cycleLengthExtension);
      }
    }
  }

  // skewed down (non-default, less likely to be correct voltage)
  else {
    for (int cycle = 0; cycle < _cycleCount; cycle++) {

      // run i++ pulse loop
      for (int i = 0; i < 255; i++) {

        for (auto iter = _analogPins.begin(); iter != _analogPins.end(); iter++) {
          uint8_t pin = iter->first;
          uint8_t value = iter->second;
          if (value > i) {
            digitalWrite(pin, HIGH);
          }
          else {
            digitalWrite(pin, LOW);
          }
        }//*/

        delayMicroseconds(_cycleLengthExtension);
      }
    }
  }//*/
}

void AnalogOutputEmulator::RunLoop() {
  while (true) {
    Run();
  }
}
