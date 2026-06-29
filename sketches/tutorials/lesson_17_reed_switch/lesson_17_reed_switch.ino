// Reed switch module - DO→D3, AO→A0, LED on D13
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
  int analogVal = analogRead(sensorPin);
  if (val == HIGH) {
    digitalWrite(Led, HIGH);
    Serial.print("Magnet detected | analog: ");
  } else {
    digitalWrite(Led, LOW);
    Serial.print("No magnet | analog: ");
  }
  Serial.println(analogVal);
  delay(200);
}
