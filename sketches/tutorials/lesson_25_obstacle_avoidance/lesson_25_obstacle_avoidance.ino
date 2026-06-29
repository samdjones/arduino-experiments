// Obstacle avoidance - OUT pin → D3 (active LOW = obstacle), LED on D13
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
  if (val == LOW) {
    digitalWrite(Led, HIGH);
    Serial.println("Obstacle detected!");
  } else {
    digitalWrite(Led, LOW);
    Serial.println("Path clear");
  }
  delay(200);
}
