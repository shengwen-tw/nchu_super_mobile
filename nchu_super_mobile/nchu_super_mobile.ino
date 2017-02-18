#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"
#include "af_guage.hpp"
#include "inject_correct.hpp"

void setup() {
  engine_rpm_init();
  stepper_init();
  dac_init();

  Serial.begin(57600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
}

void loop()
{
  float af_ratio = 0.0;
  float engine_rpm;
  bool get_rpm = get_engine_rpm(&engine_rpm);

#if 0 /* RPM test */
  if(get_rpm) {
    //Serial.print("Engine RPM: ");
    Serial.println(engine_rpm);
    delay(9);
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
    delay(1);
  }
#endif

#if 0 /* A/F Guage test */
  af_ratio = read_air_fuel_ratio();
  
  Serial.println(af_ratio);
  delay(1);
#endif

#if 0 /* Step motor test */
  stepper_motor_control(STEPPER_CW, 360, 1000); //Range from 0 to 360
  delay(1); STEPPER_SINGLE_STEP
  while(1);
#endif

#if 1 /* DAC test */
  set_dac(2.5);
#endif
}
