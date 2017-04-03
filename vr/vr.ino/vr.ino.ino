void setup() {
// put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A5, INPUT);
  pinMode(7, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int vr_width = analogRead(A5);
  int vr_freq =  analogRead(A1);

  float freq_percent = 1.0f + (vr_freq / 1023.0f) * 3.0f; //100% (15hz) to 400% (50hz)
  
   //Set the Engine speed.//
  float rpm = 1800.0f * freq_percent; //(R.P.M.), 15hz * scaler

  //Set the maximum injection time.//
  float maxinjtime=15;//(ms) Maximum = 16.385

  float Th = vr_width / 1023.0f *maxinjtime*1000.0f;//(us)
  float period=1*1000.0f/(rpm/2.0f/60.0f);//(ms)

  digitalWrite(7, HIGH);
  delayMicroseconds(Th);
  digitalWrite(7, LOW);
  delay(period-Th/1000.0f);

  //Turn off serial print to minimize the error of the width of the low level the of the signal.
  //Serial.print("Th:");
  //Serial.println(Th);
}
