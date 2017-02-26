#include "Arduino.h"

float read_air_fuel_ratio()
{ 
  if (Serial3.available() <= 0) {
    return -1;
  }

  float af_ratio = 0.0f;
  String af_raw_message = "";
  float temp = 0.0f;

  af_raw_message = Serial3.readStringUntil('\r');
  Serial3.readStringUntil('\n');
  
  temp = af_raw_message.toFloat();
  
  if (temp >= 10.0f && temp <= 19.98f) {
    af_ratio = temp;
    //Serial.print("A/F: ");
    //Serial.println(af_ratio);
    delay(1);
  } else {
    af_ratio = -1;
  }

  //Serial.println(af_ratio);
  
  return af_ratio;
}
