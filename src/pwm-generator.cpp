#include "pwm-generator.hpp"

PWMGenerator::PWMGenerator() {
  _cycleCount = 4;
  _cycleLengthExtension = 8;
  _maxDutyValue = 255;
  _queued = false;
  _skew = LOW;
}

void PWMGenerator::SetVoltage(uint8_t pin, uint16_t value) {
  _mutex.lock();
  _insertionQueue.emplace(pin, value);
  _queued = true;
  _mutex.unlock();
}

void PWMGenerator::SetCycleCount(uint16_t cycleCount) {
  _mutex.lock();
  _cycleCount = cycleCount;
  _mutex.unlock();
}

void PWMGenerator::SetCycleLengthExtension(uint16_t cycleLengthExtension) {
  _mutex.lock();
  _cycleLengthExtension = cycleLengthExtension;
  _mutex.unlock();
}

void PWMGenerator::SetMaxDutyValue(uint16_t maxDutyValue) {
  _mutex.lock();
  _maxDutyValue = maxDutyValue;
  _mutex.unlock();
}

void PWMGenerator::SetSkew(bool skew) {
  _mutex.lock();
  _skew = skew;
  _mutex.unlock();
}

void PWMGenerator::Run() {
  _mutex.lock();

  // push queued changes to main _analogPins map
  if (_queued) {
    uint8_t pin;
    uint16_t value;
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
  
  // ensures these values do not change the pwm signal mid-period
  int cycleCount = _cycleCount;
  int cycleLengthExtension = _cycleLengthExtension;
  int maxDutyValue = _maxDutyValue;
  bool skew = _skew;

  _mutex.unlock();

  // skewed HIGH (non-default, usually closer to accurate, but really bad at low voltages when there is variable gap time)
  if (_skew) {
    for (int cycle = 0; cycle < cycleCount; cycle++) {

      // i-- pulse loop
      for (int i = maxDutyValue - 1; i >= 0; i--) {
        
        for (auto iter = _analogPins.begin(); iter != _analogPins.end(); iter++) {
          uint8_t pin = iter->first;
          uint16_t value = iter->second;
          // ends on HIGH, as any value > 0 will be > i when i = 0
          if (value > i) {
            digitalWrite(pin, HIGH);
          }
          else {
            digitalWrite(pin, LOW);
          }
        }

        delayMicroseconds(cycleLengthExtension);
      }
    }
  }

  // skewed LOW (default, usually slightly less accurate, but doesn't have as big of skew issues at the extremes)
  else {
    for (int cycle = 0; cycle < cycleCount; cycle++) {

      // i++ pulse loop
      for (int i = 0; i < maxDutyValue; i++) {

        for (auto iter = _analogPins.begin(); iter != _analogPins.end(); iter++) {
          uint8_t pin = iter->first;
          uint16_t value = iter->second;
          // ends on LOW, as any value < maxDutyValue will be <= i when i = maxDutyValue - 1
          if (value > i) {
            digitalWrite(pin, HIGH);
          }
          else {
            digitalWrite(pin, LOW);
          }
        }

        delayMicroseconds(cycleLengthExtension);
      }
    }
  }
}

void PWMGenerator::RunLoop() {
  while (true) {
    Run();
  }
}
