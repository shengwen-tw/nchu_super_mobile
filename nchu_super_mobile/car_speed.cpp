#include <Arduino.h>
#include "pindef.hpp"

#define WHEEL_DIAMETER 530 //mm
#define _PI_ 3.1415926

/* Frequency counter */
volatile unsigned long car_speed_current_rising_time = 0;
volatile unsigned long car_speed_previous_rising_time = 0;
volatile unsigned long car_speed_duration = 0;
volatile unsigned long last_get_time = 0;
volatile float _car_speed = 0.0f;
boolean car_speed_trigger = false;
boolean car_speed_get = false;

boolean car_timer_trigger = false;

void car_speed_freq_calc();

void car_speed_in_init()
{
  pinMode(CAR_SPEED_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CAR_SPEED_PIN), car_speed_freq_calc, RISING);
}

void car_speed_freq_calc()
{  
  if(car_timer_trigger = false) {
    car_speed_previous_rising_time = micros();
    car_timer_trigger = true;
  } else {
    car_timer_trigger = false;
    
    car_speed_current_rising_time = micros();
    car_speed_duration = car_speed_current_rising_time - car_speed_previous_rising_time;
    
    _car_speed = 1.0f / ((float)car_speed_duration / 1000000.0f); //Convert from microsecond to millisecond
    
    _car_speed /= 48.0;
    _car_speed = 3.1415926 * WHEEL_DIAMETER * _car_speed * 0.0036; //km/hr

    car_speed_previous_rising_time = car_speed_current_rising_time;

    car_speed_get = true;
    last_get_time = millis();
  }
}

boolean get_car_speed(float *car_speed)
{
  if(car_speed_get == true) {
    *car_speed = _car_speed;
    car_speed_get = false;

    return true;
  } else {
    unsigned long current_time = millis();
    /* The wheel is not spinning */
    if(current_time - last_get_time > 1000) {
      *car_speed = 0;
      last_get_time = current_time;
      return true;
    }
    
    return false;
  }
}
