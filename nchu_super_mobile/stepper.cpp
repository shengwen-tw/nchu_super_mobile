#include <Arduino.h>

#include "pindef.hpp"
#include "stepper.hpp"

void stepper_init()
{
  /* PWM Initialization */
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_PLS, OUTPUT);
  digitalWrite(STEPPER_DIR, HIGH);
}

int stepper_motor_control(int dir, int degree, int period)
{
  int count = degree * STEPPER_SINGLE_STEP / 100;
  
  while(count--) {
    //Direction control
    if(dir == STEPPER_CW) {
      digitalWrite(STEPPER_DIR, HIGH);
    } else if(dir == STEPPER_CCW) {
      digitalWrite(STEPPER_DIR, LOW);
    } else {
      return 1;
    }
    
    //Pulse control
    digitalWrite(STEPPER_PLS, HIGH);
    delayMicroseconds(period);
    digitalWrite(STEPPER_PLS, LOW);
    delayMicroseconds(period);
  }
  
  digitalWrite(STEPPER_PLS, HIGH);
  
  return 0;
}

