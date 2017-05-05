#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"
#include "engine_control.hpp"
#include "engine_temp.hpp"
#include "af_guage.hpp"
#include "inject_correct.hpp"
#include "inject_in.hpp"
#include "fan.hpp"
#include "car_speed.hpp"

//-1, no serial output
// 0:print mode
// 1:csv log mode
// 2:A/F plot
// 3:Tablet mode
#define SERIAL_PRINT_MODE 3

#define FREQUENCY 10.0f
#define DELTA_T (1.0 / FREQUENCY)
#define ECU_MAX_CORRECTION 0.54f

/* KI and KD, tune me! */
//PI Control, best: KP = 0.45, KI = 0.5
#define AF_KP +0.6f //+1.0
#define AF_KI +0.6f
#define AF_KD -0.0f //-0.8

/* Bound integrator in range of 0% to 100% */
#define I_MAX (+ECU_MAX_CORRECTION * 10.0)
#define I_MIN (-ECU_MAX_CORRECTION * 10.0)

/* Injector parameter */
#define INJECTOR_D0 0.5f//0.5f

#define D_FILTER_SIZE 2

void serial_print();
void send_onboard_parameter_to_tablet();

/* Sensor datas */
float current_af = 0.0f;
float current_inject_duration = 0.0f;
float engine_rpm = 0.0f;
unsigned long previous_read_time = 0;
boolean sensor_failed = true;
float engine_temp;
float car_speed;

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

//D moving avearge
float d_moving_average[D_FILTER_SIZE] = {0};
int d_moving_average_count = 0;

void setup() {
  Serial.begin(115200); //Debug only
  Serial1.begin(115200);  //USB, to tablet
  Serial2.begin(9600);  //Not using
  Serial3.begin(9600);  //A/F Guage

  engine_control_init();
  engine_rpm_init();
  inject_in_init();
  car_speed_in_init();
  stepper_init();
  engine_temp_init();
  dac_init();
  fan_init();

  set_dac(2.0f);
  
  Serial.println("[Begin to check all sensors...]");

  Serial.println("[Inject period...]");
  while(get_inject_duration(&current_inject_duration) == false);
  Serial.println("Passed");

  Serial.println("[A/F guage...]");
  while(read_air_fuel_ratio(&current_af) == false);
  Serial.println("Passed");
  
  Serial.println("[Engine RPM...]");
  while(get_engine_rpm(&engine_rpm) == false);
  Serial.println("Passed");

  Serial.println("[All sensors passed!]");

  Serial.println("[Start controller!]");

  pinMode(FREQUENCY_TEST_PIN, OUTPUT);
  digitalWrite(FREQUENCY_TEST_PIN, LOW);

  previous_read_time = millis();
}

boolean read_sensors()
{
  
  int timeout_af = 65535, timeout_ij = 65535, timeout_car_speed = 65535;
  boolean get_af = false, get_inject = false, _get_car_speed = false;
  while(timeout_af--) {
    get_af = read_air_fuel_ratio(&current_af);

    if(get_af == true) {
      break;
    }
  }
  
  while(timeout_ij--) {
    get_inject = get_inject_duration(&current_inject_duration);

    if(get_inject == true && current_inject_duration > 1.0f) {
      break;
    }
  }

  while(timeout_car_speed--) {
    _get_car_speed = get_car_speed(&car_speed);

    if(_get_car_speed == true) {
      break;
    }
  }

  get_engine_rpm(&engine_rpm);

  read_engine_temp(&engine_temp);

  unsigned long current_read_time = millis();
  
  /* Successfully get the data from sensor */
  if(timeout_af != 0 && timeout_ij != 0) {
    sensor_failed = false;
    previous_read_time = current_read_time;
    return true; //No error!
  } else {
    Serial.println("Sensor failed");
    
    /* Timeout! Sensor error! */
    sensor_failed = true;

    /* If still can't get data over 500ms, then there might exist some problems! */
    if(previous_read_time - previous_read_time > 500) {
      no_previous_data = true;
      current_af = 0.0f;
      current_inject_duration = 0.0f;
      engine_rpm = 0.0f;
    }

    return false; //Sensor failed
  }
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
    set_dac(2.0f);

    return; //Leave!!!
  }
  
  //af_setpoint = 16.5f;
  current_error = (1.0f / af_setpoint) - (1.0f / current_af);
  
  p_term = AF_KP * current_error * current_af;
  i_term = 0.0f;
  d_term = 0.0f;

  if(no_previous_data == false) {
    //I term
    integrator = integrator + AF_KI * (current_af * current_error * DELTA_T);
    bound(I_MAX, I_MIN, &integrator);
    i_term = integrator;

    /* Ready to use moving average */
    if(d_moving_average_count == D_FILTER_SIZE) {
      /* D term = Kd * AFc * (now - last) / delta_t */

      //Calculate "last" using moving average
      float last = 0;
      for(int i = 0; i < d_moving_average_count; i++) {
        last += d_moving_average[i];
      }
      last /= d_moving_average_count;

      //Update moving average array and calculate "now" term
      float _now = 0;
      for(int i = 0; i < d_moving_average_count - 1; i++) {
        d_moving_average[i] = d_moving_average[i + 1]; //update except for last term
        _now += d_moving_average[i];
      }
      
      d_moving_average[d_moving_average_count - 1] = current_error; //update last term
      _now += d_moving_average[d_moving_average_count - 1]; //Add last term
      _now /= d_moving_average_count;

      d_term = AF_KD * (_now - last) / DELTA_T;
      d_term *= current_af;
    } else {
      /* Fill the moving average array */
      d_moving_average[d_moving_average_count] = current_error;
      d_moving_average_count++;
    }
  } else {
    /* Reset I and D */
    integrator = 0.0f;
    d_moving_average_count = 0;
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

void engine_auto_turn_off()
{
  if(engine_rpm > 6500.0) {
      engine_off();
  }
}

void fan_control()
{
  
}

void loop()
{ 
  boolean get_sensor_data = read_sensors();

  /* Read sensors */
  if(get_sensor_data == false && no_previous_data) {
    /* Wait for too long, there mus be some error! */
    Serial.println("[Sensor failed]");
    delay(1);
  }

  /* Call controller every 10hz */
  unsigned long current_control_time = millis();
  if(current_control_time - previous_control_time >= (unsigned long)(DELTA_T * 1000.0)) {
    air_fuel_ratio_control();
    //engine_auto_turn_off();
    fan_control();
    previous_control_time = current_control_time; //Update frequency control timer

    //Toogle the frequency test pin, remember to x2 to get the real frequency!
    //digitalWrite(FREQUENCY_TEST_PIN, !digitalRead(FREQUENCY_TEST_PIN));
  }

  /* Debug print */
  //if(get_sensor_data == true) {
    serial_print();
  //}
}

void send_onboard_parameter_to_tablet()
{
  int engine_turn_off = 0;
  int engine_turn_on = 0;
  
  char buffer[256] = {0};

  if(current_af < 10.0f) {
    sprintf(buffer, "@%04.0f%03.0f0%02.1f%03.0f%1d%1d\n",
           engine_rpm, car_speed, current_af, engine_temp, engine_turn_off, engine_turn_on);  
  } else {
    sprintf(buffer, "@%04.0f%03.0f%02.1f%03.0f%1d%1d\n",
           engine_rpm, car_speed, current_af, engine_temp, engine_turn_off, engine_turn_on);
  }
  
  Serial.print(buffer);
  Serial1.print(buffer);
  delay(1);
}

void serial_print()
{ 
   char buffer[256] = {'\0'};
  
#if (SERIAL_PRINT_MODE == 0)
   sprintf(buffer,"A/F:%.3f,error:%+.3f,last error:%+.3f,int:%+.3f,corr:%+.3f,V:%.3f,P:%+.3f,I:%+.3f,D:%+.7f,ij:%.3f,time:%.3f\n\r",
     current_af,
     current_error,
     previous_error,
     integrator,
     correction,
     dac_output_voltage,      //unit: voltage
     p_term,
     i_term,
     d_term,
     current_inject_duration, //unit: millisecond
     millis() / 1000.0f       //unit: second
    );
    Serial.print(buffer);
    delay(5);
#elif (SERIAL_PRINT_MODE == 1)
  sprintf(buffer, "%.3f,%+.3f,%+.3f,%+.3f,%+.3f,%.3f,%+.3f,%+.3f,%+.7f,%.3f,%.3f\n",
    current_af,
    current_error,
    previous_error,
    integrator,
    correction,
    dac_output_voltage,      //unit: voltage
    p_term,
    i_term,
    d_term,
    current_inject_duration, //unit: millisecond
    millis() / 1000.0f       //unit: second
  );
  Serial.print(buffer);
  delay(5);
#elif (SERIAL_PRINT_MODE == 2)
  sprintf(buffer, "%f,%f\n", current_af, 16.5f);
   Serial.print(buffer);
   delay(5);
#elif (SERIAL_PRINT_MODE == 3)
  send_onboard_parameter_to_tablet();
#endif
}
