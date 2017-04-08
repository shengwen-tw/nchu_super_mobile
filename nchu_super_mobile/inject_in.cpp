#include <Arduino.h>

#define INJECT_PIN 7

volatile unsigned long inject_in_start_time = 0;
volatile unsigned long inject_in_end_time = 0;
volatile unsigned long inject_rising_duration = 0;
boolean inject_in_trigger = false;

boolean first_get = false;
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
  } else if(inject_in_trigger == true && state == false) {
    inject_in_trigger = false;
    inject_in_end_time = micros();
    inject_rising_duration = inject_in_end_time - inject_in_start_time;

    get_time = millis();
    first_get = true;
  }
}

boolean get_inject_duration(float *high_duration)
{
  /* Timeout protection */
  if(millis() - get_time > 1000.0 || inject_rising_duration <= 500 || first_get == false) {
    inject_rising_duration = 0.0;
    return false;
  }
  
  *high_duration = (float)inject_rising_duration / 1000.0f; //Convert to millisecond
  return true; //Succeed
}


