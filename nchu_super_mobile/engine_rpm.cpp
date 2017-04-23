#include <Arduino.h>

#include "pindef.hpp"
#include "engine_rpm.hpp"

#define FILTER_SIZE 15
#define MAX_RATE 85 //hz per second
#define TIMEOUT 200 //ms

/* Frequency counter */
volatile unsigned long current_rising_time;
volatile unsigned long previous_rising_time;
volatile float new_freq;
boolean trigger = false;

/* Frequency filter */
volatile float freq_queue[FILTER_SIZE];
volatile float filtered_freq;
volatile int queue_size = 0;
volatile unsigned filter_current_time = 0;
volatile unsigned filter_prior_time = 0;
volatile float filter_prior_freq;

void rpm_signal_rising_handler()
{
  if(digitalRead(SPARK_IN) != HIGH) {
    return;
  }

  volatile unsigned long duration;
  //volatile float new_freq;
  
  if(trigger == false) {
    previous_rising_time = micros();
    trigger = true;
  } else {
    current_rising_time = micros();

    duration = current_rising_time - previous_rising_time;
    previous_rising_time = current_rising_time;
    new_freq = 1.0 / (duration / 1000000.0);
    //trigger = false;

    filter_current_time = millis();
    volatile unsigned long filter_duration = (filter_current_time - filter_prior_time) / 1000.0;
    filter_prior_time = filter_current_time;

    /* Frequency thresholding */
    if(new_freq > 130 /*|| (new_freq - filter_prior_freq) / filter_duration > 85*/) {
      return;
    }

    /* Moving average */
    /* Queue */
    if(queue_size < FILTER_SIZE) {
      queue_size++;
    }
    
    for(int i = queue_size - 1; i > 0; i--) {
      freq_queue[i] = freq_queue[i - 1];
    }
    freq_queue[0] = new_freq;

    /* Frequency mean */
    filtered_freq = 0.0;
    for(int i = 0; i < queue_size; i++) {
      filtered_freq += freq_queue[i];
    }

    filtered_freq /= queue_size;

    filter_prior_freq = filtered_freq;
  }
}

void engine_rpm_init()
{
  pinMode(SPARK_IN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SPARK_IN), rpm_signal_rising_handler, CHANGE);
}

bool get_engine_rpm(float *rpm)
{
  *rpm = filtered_freq * 60;
  return true;
}

void engine_rpm_test()
{
  float engine_rpm;
  bool get_rpm = get_engine_rpm(&engine_rpm);
  if(get_rpm) {
    Serial.print("Engine RPM: ");
    Serial.println(engine_rpm);
    delay(1);
  }
}

