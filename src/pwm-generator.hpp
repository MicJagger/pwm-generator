#include <Arduino.h>

#include <map>
#include <mutex>

// keeps a list of pins and their duty values to output analog voltages
// this should be ran on its own as much as possible, as it is timing sensitive
// recommended to Run() alone in loop() or RunLoop() pinned to its own core (1 on esp32), and have (nearly) everything else run on a separate core
class PWMGenerator {

  // list of pins and their duty value out of maxDutyValue
  // pin number, voltage value
  std::map<uint8_t, uint16_t> _analogPins;

  // list of pins and their duty value out of maxDutyValue that have been recently added, but are not committed yet
  // pin number, voltage value
  std::map<uint8_t, uint16_t> _insertionQueue;

  // keeps track if anything is in _insertionQueue (simpler than checking .begin() != .end())
  bool _queued;

  // locks variables at the start of most functions, and unlocks them when they are no longer accessing / changing them
  std::mutex _mutex;

  // default = 4, setting to 0 disables PWMs but not queuing updates
  uint16_t _cycleCount;

  // extends the length of the each pulse by n microseconds
  // default = 8
  uint16_t _cycleLengthExtension;

  // maximum value that dictates the number of possible divisions for voltage
  // default = 255
  uint16_t _maxDutyValue;

  // determines resting values between pulse periods
  // default = LOW
  bool _skew;

public:

  PWMGenerator();

  // Settings

  // controls how many cycles are done in each execution of Run()
  // minimizes the gaps inbetween executions (other tasks, function overhead)
  // 
  // increases the execution time of Run() roughly linearly
  // setting to 0 disables PWMs, so all pins (!= 0, != maxDutyValue) will default to HIGH or LOW based on SetSkew (default = HIGH)
  // with PWMs disabled at value 0 will still allow you to change all values, just the outputs will be disabled
  // CAUTION: HIGH CYCLE LENGTH AND/OR HIGH CYCLE COUNT MAY CRASH YOUR MICROCONTROLLER DUE TO WATCHDOG
  // 
  // default = 4
  void SetCycleCount(uint16_t cycleCount);

  // extends the length of a pulse period
  // for example, instead of 2 us HIGH + 8 us LOW, could become 4 us HIGH + 16 us LOW
  // weird timings at the microsecond level can make voltage unpredictable depending on microcontroller, not recommended to change
  // 
  // increases the execution time of Run() roughly linearly
  // If this is set too high, the pulses may get too long and you may experience improper behavior (such as LED flickering, unstable voltages)
  // CAUTION: HIGH Run() EXECUTION TIME MAY CRASH YOUR MICROCONTROLLER DUE TO WATCHDOG
  // 
  // default = 8
  void SetCycleLengthExtension(uint16_t cycleLengthExtension);

  // sets the amount of possible values that can distinguished between for setting voltages
  // in a sense, it is the denominator in the fraction of reference voltage (value / maxDutyValue) * Vref = V
  // 
  // setting this higher increases precision, but also increases execution time of Run() roughly linearly
  // CAUTION: HIGH Run() EXECUTION TIME MAY CRASH YOUR MICROCONTROLLER DUE TO WATCHDOG
  // 
  // default = 255
  void SetMaxDutyValue(uint16_t maxDutyValue);

  // after Run() commits pulses, there is a gap period where the function is finished, and potentially other functions run
  // the skew refers to what the pin will be set to after the pulse period is over (HIGH or LOW)
  // in situations with other functions running on the same loop, skew will affect the direction the voltage leans from where it expects
  // if cycleCount is set to 0, all pins will reflect this value (unless they are 0 or maxDutyValue)
  // 
  // default = LOW
  void SetSkew(bool skew);


  // Main

  // ensure pin is set to output before doing this
  // value is 0-maxDutyValue, with 0 being 0 V and maxDutyValue being Vref (in my case, ~3.28 V)
  // think of it as a fraction, where (value / maxDutyValue) * Vref = V
  // For example, with default maxDutyValue = 255, ~1 V = 78, ~2 V = 155, ~3 V = 233
  void SetVoltage(uint8_t pin, uint16_t value);

  // put this in main loop() function, and dedicate everything (or essentially everything else) to tasks on the other core(s)
  // this function is time sensitive to the proper voltage settings on the pins
  // if there is other processing to do on this core, consider changing cycleCount or cycleLengthExtension to fight skewed voltages
  void Run();

  // runs the Run() function in an indefinite while loop
  // be careful using this, as it could trigger a watchdog reset (for example, on the esp32, do not pin this on core 0)
  void RunLoop();
};
