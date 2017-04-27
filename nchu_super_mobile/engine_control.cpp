#include <Arduino.h>
#include "pindef.hpp"

void engine_control_init()
{
  pinMode(ENGINE_CONTROL, OUTPUT);
  digitalWrite(ENGINE_CONTROL, LOW);
}

void engine_off()
{
  digitalWrite(ENGINE_CONTROL, HIGH);
  delay(2000);
  digitalWrite(ENGINE_CONTROL, LOW);
}

