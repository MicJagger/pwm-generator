#include "pwm-generator.hpp"

#define pin_TEST1 23
#define pin_TEST2 22

TaskHandle_t mainTasks;
TaskHandle_t pwmTask;

PWMGenerator pwmg;

void Main(void* parameter);
void PWMTask(void* parameter);


//

void setup() {
  Serial.begin(115200);
  Serial.println("Setup Sequence Initialized");

  pinMode(pin_TEST1, OUTPUT);
  pinMode(pin_TEST2, OUTPUT);

  pwmg.SetVoltage(pin_TEST1, 127);
  pwmg.SetVoltage(pin_TEST2, 200);

  pwmg.SetCycleCount(4); // default value
  pwmg.SetCycleLengthExtension(8); // default value
  pwmg.SetMaxDutyValue(255); // default value
  pwmg.SetSkew(LOW); // default value

  // pinning this to core 0 WILL CRASH an ESP32
  xTaskCreatePinnedToCore(PWMTask, "PWMTask", 10000, NULL, 1, &pwmTask, 1);
  delay(50);

  xTaskCreatePinnedToCore(Main, "Main", 10000, NULL, 1, &mainTasks, 0);
  delay(50);

  Serial.println("Booted and Setup Successfully");
}

void loop() {
  delay(10000);
}


// 

void Main(void* parameter) {
  Serial.println("Main Task Initialized");
  while (true) {
    for (int i = 0; i < 128; i += 16) {
      pwmg.SetVoltage(pin_TEST1, i);
      delay(2000);
    }
  }
}

void PWMTask(void* parameter) {
  Serial.println("PWMTask Initialized");
  pwmg.RunLoop();
}
