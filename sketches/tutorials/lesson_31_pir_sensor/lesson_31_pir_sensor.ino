// HC-SR501 PIR - OUT→D3, LED on D13
// Sensor needs ~60s to initialize; motion triggers output HIGH
int pirPin = 3;
int ledPin = 13;
int pirState = LOW;
int val = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  Serial.begin(9600);
  Serial.println("Warming up PIR sensor (~60s)...");
}

void loop() {
  if (millis() < 60000UL) {
    return;
  }
  val = digitalRead(pirPin);
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH);
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }
  } else {
    digitalWrite(ledPin, LOW);
    if (pirState == HIGH) {
      Serial.println("Motion ended.");
      pirState = LOW;
    }
  }
}
