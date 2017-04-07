#define INJECT_PIN 7

volatile unsigned long start_time;
volatile unsigned long end_time;
boolean trigger = false;
volatile unsigned long rising_period = 0;

void setup() {
  pinMode(INJECT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INJECT_PIN), inject_period_calc, CHANGE);
  Serial.begin(57600);
}

void inject_period_calc()
{
  int state = digitalRead(INJECT_PIN);  

  if(state == HIGH) {
    trigger = true;
    start_time = millis();
  } else if(trigger == true && state == false) {
    trigger = false;
    end_time = millis();
    rising_period = end_time - start_time;
  }
}

void loop() {
  Serial.println(rising_period);
  delay(1);
}
