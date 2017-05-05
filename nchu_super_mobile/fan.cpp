#include <Arduino.h>
#include "pindef.hpp"

void fan_init()
{
  pinMode(FAN_CONTROL, OUTPUT);
  digitalWrite(FAN_CONTROL, LOW);
}

void fan_on()
{
  digitalWrite(FAN_CONTROL, HIGH);
}


void fan_off()
{
  digitalWrite(FAN_CONTROL, LOW);
}

