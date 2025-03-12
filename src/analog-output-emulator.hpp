#include <Arduino.h>

#include <map>
#include <mutex>

// Keeps a list of pins and their analog voltages as a percentage of output
// This should be ran mostly on its own, as it is timing sensitive
// Recommended to run alone in loop() and have (nearly) everything else run on a separate core
class AnalogOutputEmulator {
  // pin number, voltage value
  std::map<uint8_t, uint8_t> analogPins;
  // pin number, voltage value
  std::map<uint8_t, uint8_t> insertionQueue;

  std::mutex mtx;
public:
  // ensure pin is set to output before doing this
  // value is 0-255, with 0 being 0 V and 255 being Vout (in my case, ~3.28 V)
  void SetVoltage(uint8_t pin, uint8_t value);

  // put this in main loop() function, and dedicate everything (or essentially everything else) to tasks on the other core(s)
  void Run();
};
