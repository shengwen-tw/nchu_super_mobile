#include <Arduino.h>
#include "pindef.hpp"

void engine_temp_init()
{
  pinMode(ENGINE_TEMP, INPUT);
}

void read_engine_temp(float *engine_temp)
{
  int temp_unscaled = analogRead(ENGINE_TEMP);

  *engine_temp = -30.2f * (float)temp_unscaled / 1023.0f * 5.0f + 161.6;
  //Serial.println(*engine_temp);
}

