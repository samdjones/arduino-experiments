// Touch sensor - DO→D3, AO→A0, LED on D13
int Led = 13;
int buttonpin = 3;
int sensorPin = A0;
int val;

void setup() {
  pinMode(Led, OUTPUT);
  pinMode(buttonpin, INPUT);
  Serial.begin(9600);
}

void loop() {
  val = digitalRead(buttonpin);
  int sensorValue = analogRead(sensorPin);
  if (val == HIGH) {
    digitalWrite(Led, HIGH);
  } else {
    digitalWrite(Led, LOW);
  }
  Serial.println(sensorValue, DEC);
  delay(50);
}
