#include <Arduino.h>
#include "pindef.hpp"

void engine_temp_init()
{
  pinMode(ENGINE_TEMP, INPUT);
}

void read_engine_temp(float *engine_temp)
{
  int temp_unscaled = analogRead(ENGINE_TEMP);

  *engine_temp = (float)temp_unscaled;
}

