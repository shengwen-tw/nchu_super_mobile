#include <Arduino.h>
#include "pindef.hpp"

volatile unsigned long inject_in_start_time = 0;
volatile unsigned long inject_in_end_time = 0;
float inject_rising_duration = 0;
boolean inject_in_trigger = false;

boolean _ready = false;
volatile unsigned long get_time = 0;

void inject_duration_calc();

void inject_in_init()
{
  pinMode(INJECT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INJECT_PIN), inject_duration_calc, CHANGE);
}

void inject_duration_calc()
{
  int state = digitalRead(INJECT_PIN);  

  /* Record the start time at rising edge */
  if(state == HIGH) {
    inject_in_trigger = true;
    inject_in_start_time = micros();
  /* Recode the end time at falling edge */
  } else if(state == LOW && inject_in_trigger == true) {
    inject_in_trigger = false;
    inject_in_end_time = micros();
    
    volatile unsigned long temp = inject_in_end_time - inject_in_start_time;

    if(temp < 2) {
      _ready = false;
    } else {
      get_time = millis();
      _ready = true;
      inject_rising_duration = (float)temp;
      inject_rising_duration /= 1000.0; //Convert from microsecond to millisecond
    }
  }
}

boolean get_inject_duration(float *high_duration)
{ 
  /* Timeout protection & duration < 2 millisecond */
  if(millis() - get_time > 500 || _ready != true) {
    return false;
  } else {
    *high_duration = inject_rising_duration;
    return true; //Succeed
  }
}


