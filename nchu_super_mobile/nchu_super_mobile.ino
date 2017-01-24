#include "pindef.hpp"
#include "stepper.hpp"
#include "engine_rpm.hpp"

void setup() {
  engine_rpm_init();
  stepper_init();

  Serial.begin(57600);
}

#ifdef DEBUG_CODE
void read_spark_pulse()
{
  double duration_high =  pulseIn(SPARK_IN, HIGH);
  double duration_low = pulseIn(SPARK_IN, LOW); 
  double frequency = 1.0 / ((duration_high + duration_low) / 1000000);
  //delay(500);
  
  Serial.print("Engine RPM:");
  Serial.println(frequency);
}
#endif

void loop()
{
  float engine_rpm;
  bool get_rpm = get_engine_rpm(&engine_rpm);

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

  analogWrite(INJECT_CORRECTION, 255);
  
  //stepper_motor_control(STEPPER_CW, 360, 1000); //Range from 0 to 360
  //delay(1); STEPPER_SINGLE_STEP
  //while(1);
}
