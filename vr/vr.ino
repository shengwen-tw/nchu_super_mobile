void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A5, INPUT);
  pinMode(9, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int vr = analogRead(A5);
  float percent = vr / (float)1024 / 10;
  digitalWrite(9, HIGH);
  delayMicroseconds(percent * 50 * 1000);
  digitalWrite(9, LOW);
  delayMicroseconds((1.0f - percent) * 50 * 1000);

  Serial.print("On period:");
  Serial.println(percent);
}
