#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"
#include "af_guage.hpp"
#include "inject_correct.hpp"
#include "interrupt.hpp"

#define AF_P 0.5
#define AF_I 0.0015
#define AF_D 0.0
#define DELTA_T 10.0 //ms
#define I_ATTENUATE 0.98

void driver_test();

/* Sensor data */
float current_af = 0.0f;
float engine_rpm = 0.0f;
bool sensor_failed = false;
unsigned long previous_read_time = 0;

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
  timer_interrupt_init();

  do {
    current_af = read_air_fuel_ratio();
  } while(current_af < 10.0f || current_af > 19.98);

#if 0
  do {
    bool get_rpm = get_engine_rpm =(&engine_rpm);
  } while(get_rpm == false);
#endif
  previous_read_time = millis();

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
    set_dac(2.0);
  }
  
  /* Air/Fuel ratio PID control */
  float af_setpoint = 14.7;
  float current_error = af_setpoint - current_af;

  float p_term = AF_P * current_error;
  float _integral = 0.0f;
  float i_term = 0.0f;
  float d_term = 0.0f;

  if(no_previous_data == false) {
    _integral = I_ATTENUATE * (integral + current_error * DELTA_T);
    i_term = AF_I * _integral;
    d_term = AF_D * (current_error - previous_error) / DELTA_T;
  }

  /* Update datas for next iteration */
  no_previous_data = false;
  previous_error = current_error;
  integral = _integral;

  /* Set DAC voltage */
  float dac_output_voltage = 2.0f + -(p_term + i_term + d_term);
  bound(4.0, 0.0, &dac_output_voltage);
  set_dac(dac_output_voltage);

  //Serial.println(current_af);
  Serial.println(dac_output_voltage - 2.0f);
  //Serial.println(i_term);
}

void TC0_Handler()
{
  REG_TC0_SR0; //Clear timer counter

  air_fuel_ratio_control();
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

  float temp_af = read_air_fuel_ratio();
  if(temp_af != -1) {
    current_af = temp_af;
    previous_read_time = millis();
    sensor_failed = true;
  } else {
    if(millis() - previous_read_time > 200) {
      sensor_failed = false;
      no_previous_data = true;
    }
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
