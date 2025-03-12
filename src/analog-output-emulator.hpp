#include <Arduino.h>

#include <map>
#include <mutex>

// Keeps a list of pins and their analog voltages as a percentage of output
// This should be ran mostly on its own, as it is timing sensitive
// Recommended to run alone in loop() and have (nearly) everything else run on a separate core
class AnalogOutputEmulator {

  // pin number, voltage value
  std::map<uint8_t, uint8_t> _analogPins;

  // 
  std::map<uint8_t, uint8_t> _insertionQueue;
  bool _queued;

  // 
  std::mutex _mutex;

  // default = 1, setting to 0 disables PWMs but not queuing updates
  uint16_t _cycleCount;

  // extends the length of the each pulse by n microseconds
  // useful for having a longer cycle, which fights voltage inaccuracy if the thread has other work to do
  // default = 1
  uint16_t _cycleLengthExtension;

  // default = true
  bool _skewHIGH;

public:

  AnalogOutputEmulator();

  // Settings

  // controls how many cycles are done in each execution of Run()
  // increases the execution time of Run() roughly linearly 
  // most useful to ensure more stable voltages if needed for some reason
  // CAUTION: MIXING HIGH CYCLE LENGTH WITH CYCLE COUNT MAY CRASH YOUR MICROCONTROLLER DUE TO WATCHDOG
  // default = 4
  void SetCycleCount(uint16_t cycleCount);

  // extends the length of a pulse period
  // increases the execution time of Run() roughly linearly
  // If this is set too high, the pulses may get too long and you may experience improper behavior (such as LED flickering)
  // CAUTION: MIXING HIGH CYCLE LENGTH WITH CYCLE COUNT MAY CRASH YOUR MICROCONTROLLER DUE TO WATCHDOG
  // default = 8
  void SetCycleLengthExtension(uint16_t cycleLengthExtension);

  // Inbetween one pulse period, pulses are normally left on HIGH
  // this setting will leave pulses on LOW between Run() runs if you so want to do that
  // default = true
  void SetSkew(bool skewHIGH);


  // Main

  // ensure pin is set to output before doing this
  // value is 0-255, with 0 being 0 V and 255 being Vout (in my case, ~3.28 V)
  void SetVoltage(uint8_t pin, uint8_t value);

  // put this in main loop() function, and dedicate everything (or essentially everything else) to tasks on the other core(s)
  // this function is time sensitive to the proper voltage settings on the pins
  // if there is a reasonable amount of other processing to do on this core, consider changing cycleCount or cycleLengthExtension
  void Run();

  // runs the Run() function in an indefinite while loop
  // Be careful using this, as it could trigger a watchdog reset (esp32, do not on core 0)
  void RunLoop();
};
