#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"
#include "af_guage.hpp"
#include "inject_correct.hpp"
#include "interrupt.hpp"

#define AF_P 0.01
#define AF_I 0.0
#define AF_D 0.0
#define DELTA_T 10.0 //ms

void driver_test();

/* Sensor data */
float current_af = 0.0f;
float engine_rpm = 0.0f;
bool sensor_failed = false;

/* Controller data */
float previous_error = 0.0;
float integral = 0.0;
bool no_previous_data = true;

void setup() {
  Serial.begin(57600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  
  engine_rpm_init();
  stepper_init();
  dac_init();
  //timer_interrupt_init();

  do {
    current_af = read_air_fuel_ratio();
  } while(current_af < 10.0f || current_af > 19.98);

#if 0
  do {
    bool get_rpm = get_engine_rpm(&engine_rpm);
  } while(get_rpm == false);
#endif

  Serial.println("[Air/Fuel Controller pass test]\n");
}

void bound(float upper_bound, float lower_bound, float *x)
{
  if(*x > upper_bound) {
    *x = upper_bound;
  } else if(*x < lower_bound) {
    *x = lower_bound;
  }
}

void air_fuel_ratio_control()
{
  if(sensor_failed == true) {
    set_dac(2.5);
  }
  
  /* Air/Fuel ratio PID control */
  float af_setpoint = 14.7;
  float current_error = af_setpoint - current_af;

  float p_term = AF_P * current_error;

  float i_term = 0.0f;
  float d_term = 0.0f;

  if(no_previous_data == false) {
    i_term = AF_I * (integral + current_error);
    d_term = AF_D * (current_error - previous_error);
  }

  /* Update datas for next iteration */
  no_previous_data = false;
  previous_error = current_error;
  integral = i_term;

  /* Set DAC voltage */
  float dac_output_voltage = 2.5f + p_term + i_term + d_term;
  bound(5.0, 0.0, &dac_output_voltage);
  set_dac(dac_output_voltage);
}

void TC0_Handler()
{
  REG_TC0_SR0; //Clear timer counter

  toogle_freq_test_pin();
}

void loop()
{
#if 0 /* RPM */
  bool get_rpm = get_engine_rpm(&engine_rpm);
  if(get_rpm) {
    Serial.print("Engine RPM: ");
    Serial.println(engine_rpm);
    delay(9);
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
    delay(1);
  }
#endif

  current_af = read_air_fuel_ratio();
  if(current_af == -1) {
    sensor_failed = true;
  } else {
    sensor_failed = false;
  }
}

void driver_test()
{
#if 0 /* Step motor test */
  stepper_motor_control(STEPPER_CW, 360, 1000); //Range from 0 to 360
  delay(1); STEPPER_SINGLE_STEP
  while(1);
#endif
}
