#define SPARK_IN 2
#define STEPPER_DIR 11
#define STEPPER_PLS 10

#define STEPPER_RESOLUTION 0.9
#define STEPPER_SINGLE_STEP (1 / STEPPER_RESOLUTION)

void frequency_counter_handler();

enum StepperDirection {STEPPER_CW, STEPPER_CCW};

volatile unsigned long current;
volatile unsigned long previous;
volatile unsigned long duration;
volatile float frequency;
boolean trigger = false;

void setup() {
  pinMode(SPARK_IN, INPUT);
  attachInterrupt(0, frequency_counter_handler, CHANGE);
  
  /* PWM Initialization */
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_PLS, OUTPUT);
  digitalWrite(STEPPER_DIR, HIGH);

  Serial.begin(9600);
}

void read_spark_pulse()
{
  double duration_high =  pulseIn(SPARK_IN, HIGH);
  double duration_low = pulseIn(SPARK_IN, LOW); 
  double frequency = 1.0 / ((duration_high + duration_low) / 1000000);
  //delay(500);
  
  Serial.print("Engine RPM:");
  Serial.println(frequency);
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

void frequency_counter_handler()
{
  if(digitalRead(SPARK_IN) == HIGH) {
    if(trigger == false) {
      previous = micros();
      trigger = true;
    } else {
      current = micros();

      duration = current - previous;
      frequency = 1.0 / (duration / 1000000.0);

      previous = current;
      //trigger = false;
    }
  }
}

void print_frequency()
{
  if(frequency < 130) {
    Serial.print("Frequency: ");
    Serial.println(frequency);
    delay(10);
  }
}

void loop()
{
  print_frequency();
  //stepper_motor_control(STEPPER_CW, 360, 1000); //Range from 0 to 360
  //delay(1); STEPPER_SINGLE_STEP
  //while(1);
}
