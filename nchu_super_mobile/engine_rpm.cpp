#include <Arduino.h>

#include "pindef.hpp"
#include "engine_rpm.hpp"

volatile unsigned long current;
volatile unsigned long previous;
volatile unsigned long duration;
volatile float frequency;
boolean trigger = false;

void frequency_counter_handler()
{
  if(digitalRead(SPARK_IN) == HIGH) {
    if(trigger == false) {
      previous = micros();
      trigger = true;
    } else {
      current = micros();

      duration = current - previous;
      frequency = 1.0 / (duration / 1000000.0);

      previous = current;
      //trigger = false;
    }
  }
}

void engine_rpm_init()
{
  pinMode(SPARK_IN, INPUT);
  attachInterrupt(0, frequency_counter_handler, RISING);
}

float get_engine_rpm()
{
  if(frequency < 130) {
    Serial.print("Frequency: ");
    Serial.println(frequency);
    delay(10);
  }

  return frequency * 60;
}
