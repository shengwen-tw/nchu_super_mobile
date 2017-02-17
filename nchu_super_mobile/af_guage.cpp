#include "Arduino.h"

float af_ratio = 0;

float read_air_fuel_ratio()
{
  String af_raw_message = "";
  float temp = 0;

  if (Serial3.available())
  {
    af_raw_message = Serial3.readStringUntil('\n');
    Serial3.readStringUntil('\r');
  }
  
  temp = af_raw_message.toFloat();
  
  if (temp >= 6 /*&& temp < X*/) {
    af_ratio = temp;
  }

  return af_ratio;
}
