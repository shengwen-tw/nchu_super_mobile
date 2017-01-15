#define SPARK_IN 2
#define STEPPER_DIR 11
#define STEPPER_PLS 10

#define STEPPER_RESOLUTION 0.9
#define STEPPER_SINGLE_STEP (1 / STEPPER_RESOLUTION)

enum StepperDirection {STEPPER_CW, STEPPER_CCW};

void setup() {
  pinMode(SPARK_IN, INPUT);
  
  /* PWM Initialization */
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_PLS, OUTPUT);
  digitalWrite(STEPPER_DIR, HIGH);

  Serial.begin(9600);
}

void read_spark_pulse()
{
  unsigned long time_before = millis();
  int counter = 0;
  int state = -1;
  while((millis() - time_before) < 1000) {
    if(state == -1) {
      while((state = digitalRead(SPARK_IN)) != 0);
    }

    delay(5);

    if(digitalRead(SPARK_IN) == 1) {
      counter++;
      state = -1;
    }
  }

  Serial.print("Frequency:");
  Serial.println(counter);
}

int stepper_motor_control(int dir, int degree, int period)
{
  int count = degree * STEPPER_SINGLE_STEP;
  
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

void loop() {
#if 0
  read_spark_pulse();
  delay(1);
#endif

  stepper_motor_control(STEPPER_CW, 360, 1000); //Range from 0 to 360
  
  while(1);
}
