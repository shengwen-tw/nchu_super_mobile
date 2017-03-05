#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"
#include "af_guage.hpp"
#include "inject_correct.hpp"
#include "interrupt.hpp"

#define AF_D 0.0
#define AF_I 0.0
#define I_MAX 2.0
#define I_MIN 0
#define DELTA_T 0.01 //ms

void driver_test();

/* Sensor data */
float current_af = 0.0f;
float engine_rpm = 0.0f;
bool sensor_failed = false;
unsigned long previous_read_time = 0;

/* Controller data */
float previous_error = 0.0;
float integrator = 0.0;
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

  float af_setpoint = 14.7;
  float current_error = (1.0 / af_setpoint) - (1.0 / current_af);

  float correction = current_af * current_error;
  float p_term = correction / 0.27f * 2.0f;
  float i_term = 0.0f;
  float d_term = 0.0f;

  if(no_previous_data == false) {
    //I term
    if(fabs(af_setpoint - current_af) < 0.3) {
        //I term is no need when the error is so small
        integrator = 0.0f;
    } else {
        integrator += (AF_I * p_term * DELTA_T);
        bound(I_MAX, I_MIN, &integrator);
    }
    i_term = integrator;

    //D term
    d_term = AF_D * (current_error - previous_error) * 10000.0 / DELTA_T;
  }

  /* next iteration */
  no_previous_data = false;
  previous_error = current_error;

  float dac_output_voltage = p_term + i_term + d_term + 2.0f;
  
  bound(4.0, 0.0, &dac_output_voltage);
  set_dac(dac_output_voltage);

  #if 0
    Serial.print("d_term:");
    Serial.print(d_term);
    Serial.print(",af:");
    Serial.print(current_af);
    Serial.print(",%:");
    Serial.print(correction);
    Serial.print(",V:");
    Serial.println(dac_output_voltage);
  #endif
  
  #if 1
    Serial.print(p_term);
    Serial.print(" ");
    Serial.println(i_term);
  #endif
}

void TC0_Handler()
{
  REG_TC0_SR0; //Clear timer counter
  toogle_freq_test_pin();
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
      integrator = 0.0;
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
