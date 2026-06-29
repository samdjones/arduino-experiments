// Line tracking - OUT pin → D3, LED on D13
// OUT goes LOW when black surface (less IR reflection)
int Led = 13;
int buttonpin = 3;
int val;

void setup() {
  pinMode(Led, OUTPUT);
  pinMode(buttonpin, INPUT);
  Serial.begin(9600);
}

void loop() {
  val = digitalRead(buttonpin);
  if (val == HIGH) {
    digitalWrite(Led, HIGH);
    Serial.println("Light surface");
  } else {
    digitalWrite(Led, LOW);
    Serial.println("Dark surface");
  }
  delay(100);
}
