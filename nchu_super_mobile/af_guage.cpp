#include "Arduino.h"

boolean read_air_fuel_ratio(float *curr_af)
{ 
  if (Serial3.available() <= 0) {
    return false;
  }

  float af_ratio = 0.0f;
  String af_raw_message = "";
  float temp = 0.0f;

  af_raw_message = Serial3.readStringUntil('\r');
  Serial3.readStringUntil('\n');
  
  temp = af_raw_message.toFloat();
  
  if (temp >= 10.0f && temp <= 19.98f) {
    af_ratio = temp;
  } else {
    return false;
  }

  //Serial.println(af_ratio);
  
  *curr_af = af_ratio;

  return true;
}

