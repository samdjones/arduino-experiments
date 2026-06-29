// Microphone module - AO→A0, DO→D7, LED on D13
const int sensorAnalogPin = A0;
const int sensorDigitalPin = 7;
const int Led13 = 13;
int analogValue = 0;
int digitalValue;

void setup() {
  Serial.begin(9600);
  pinMode(sensorDigitalPin, INPUT);
  pinMode(Led13, OUTPUT);
}

void loop() {
  analogValue = analogRead(sensorAnalogPin);
  digitalValue = digitalRead(sensorDigitalPin);
  Serial.println(analogValue);
  if (digitalValue == HIGH) {
    digitalWrite(Led13, HIGH);
  } else {
    digitalWrite(Led13, LOW);
  }
  delay(50);
}
