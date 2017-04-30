#include <Arduino.h>
#include "pindef.hpp"

void car_speed_freq_calc();

void car_speed_in_init()
{
  pinMode(CAR_SPEED_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CAR_SPEED_PIN), car_speed_freq_calc, CHANGE);
}

void car_speed_freq_calc()
{
  int state = digitalRead(CAR_SPEED_PIN);
}

boolean get_car_speed(float *car_speed)
{ 
}


