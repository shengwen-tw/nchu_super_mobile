#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"
#include "af_guage.hpp"
#include "inject_correct.hpp"
#include "inject_in.hpp"

#define LOG_MODE 0 //0:csv log mode, 1:print mode, 2:A/F plot mode

#define ECU_MAX_CORRECTION 0.54f
/* KI and KD, tune me! */
#define AF_P +0.65f //Fixed 0.65
#define AF_I +0.5f
#define AF_D -0.0f //Useless
/* Bound integrator in range of 0% to 100% */
#define I_MAX (+ECU_MAX_CORRECTION * 10.0)
#define I_MIN (-ECU_MAX_CORRECTION * 10.0)
#define DELTA_T 0.01f //ms
/* Injector parameter */
#define INJECTOR_D0 0.0f//0.5f

#define FREQUENCY 10.0f
#define DELTA_T (1.0 / FREQUENCY)

void driver_test();

/* Sensor datas */
float current_af = 0.0f;
float current_inject_duration = 0.0f;
float engine_rpm = 0.0f;
unsigned long previous_read_time = 0;
boolean sensor_failed = false;

/* Controller timer: 10hz */
unsigned long previous_control_time = 0;

/* Controller data */
float af_setpoint = 16.5f;
float current_error = 0.0f;
float p_term = 0.0f;
float i_term = 0.0f;
float d_term = 0.0f;
float injector_fix_coefficient = 0.0f;
float correction = 0.0f;
float dac_output_voltage = 0.0f;
//Timeout protection
float previous_error = 0.0;
float integrator = 0.0; //Bound in 0%~100%
bool no_previous_data = true;

void setup() {
  Serial.begin(115200); //USB, to tablet
  Serial1.begin(9600);  //Not using
  Serial2.begin(9600);  //Not using
  Serial3.begin(9600);  //A/F Guage

  engine_rpm_init();
  inject_in_init();
  stepper_init();
  dac_init();

  set_dac(2.0f);

  Serial.println("[Begin to check all sensors...]");

  Serial.println("[Inject period...]");
  while(get_inject_duration(&current_inject_duration) == false);
  Serial.println("Passed");

  Serial.println("[A/F guage...]");
  while(read_air_fuel_ratio(&current_af) == false);
  Serial.println("Passed");

#if 0
  Serial.println("[Engine RPM...]");
  while(get_engine_rpm(&engine_rpm) == false);
  Serial.println("Passed");
#endif

  Serial.println("[All sensors passed!]");

  previous_read_time = millis();

  delay(10000);

  Serial.println("[Start controller!]");

  Serial.println("Please wait for 10 seconds!");

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
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
  /* Sensor failed */
  if(sensor_failed == true) {
    set_dac(2.0f); //0%
    return;
  }
  
  //af_setpoint = 16.5f;
  current_error = (1.0f / af_setpoint) - (1.0f / current_af);
  
  p_term = AF_P * current_error * current_af;
  i_term = 0.0f;
  d_term = 0.0f;

  if(no_previous_data == false) {
    //I term
    integrator = integrator + AF_I * (current_af * current_error * DELTA_T);
    bound(I_MAX, I_MIN, &integrator);
    i_term = integrator;

    //D term
    d_term = AF_D * (current_error - previous_error) / DELTA_T;
    d_term *= current_af;
  } else {
    integrator = 0.0f;
  }

  /* next iteration */
  no_previous_data = false;
  previous_error = current_error;

  /* Correction = (P + I + D) *stop inf injector_fix_coefficient */
  injector_fix_coefficient = (current_inject_duration - INJECTOR_D0) / current_inject_duration; //(dc - d0) / dc
  correction = (p_term + i_term + d_term) * injector_fix_coefficient;

  /* Convert to DAC value */
  dac_output_voltage = correction / ECU_MAX_CORRECTION * 2.0f + 2.0f;
  bound(4.0, 0.0, &dac_output_voltage);
  set_dac(dac_output_voltage);
}

boolean read_sensors()
{
  int timeout_af = 65535, timeout_ij = 65535;
  boolean get_af, get_inject;
  while(timeout_af--) {
    get_af = read_air_fuel_ratio(&current_af);

    if(get_af == true) {
      break;
    }
  }
  
  while(timeout_ij--) {
    get_inject = get_inject_duration(&current_inject_duration);

    if(get_inject == true) {
      break;
    }
  }

  //Serial.println(current_inject_duration);delay(10);

  /* Successfully get the data from sensor */
  if(timeout_af != 0 && timeout_ij != 0) {
    previous_read_time = millis();
    return true; //No error!
  /* Timeout! Sensor error! */
  } else {
    if(millis() - previous_read_time > 2000) {
      no_previous_data = true;
    }

    return false; //Sensor failed
  }
}

void loop()
{
  //engine_rpm_test();
  if(read_sensors() == false) {
    sensor_failed = true;
    Serial.println("[Sensor failed]");
    delay(1);
    return; //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  }

  unsigned long current_control_time = millis();
  if(current_control_time - previous_control_time >= (unsigned long)(DELTA_T * 1000.0)) {
    air_fuel_ratio_control();
    previous_control_time = current_control_time;

    digitalWrite(2, !digitalRead(2));

    char buffer[256] = {'\0'};

#if (LOG_MODE == 0)
   sprintf(buffer,"A/F:%.3f,error:%+.3f,last error:%+.3f,int:%+.3f,corr:%+.3f,V:%.3f,P:%+.3f,I:%+.3f,D:%+.7f,ij:%.3f,time:%.3f\n\r",
     current_af,
     current_error,
     previous_error,
     integrator,
     correction,
     dac_output_voltage,
     p_term,
     i_term,
     d_term,
     current_inject_duration,
     millis() / 1000.0f
    );
#elif (LOG_MODE == 1)
   sprintf(buffer, "%.3f,%+.3f,%+.3f,%+.3f,%+.3f,%.3f,%+.3f,%+.3f,%+.7f,%.3f,%.3f\n",
     current_af,
     current_error,
     previous_error,
     integrator,
     correction,
     dac_output_voltage,
     p_term,
     i_term,
     d_term,
     current_inject_duration,
     millis() / 1000.0f
   );
#elif (LOG_MODE == 2)
  sprintf(buffer, "%f,%f\n", current_af, 16.5f);
#endif

    Serial.print(buffer);
    delay(5);
  }
}

void engine_rpm_test()
{
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
}

void driver_test()
{
#if 0 /* Step motor test */
  stepper_motor_control(STEPPER_CW, 360, 1000); //Range from 0 to 360
  delay(1); STEPPER_SINGLE_STEP
  while(1);
#endif
}
