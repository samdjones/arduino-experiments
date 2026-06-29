// Seven-color flash LED - S pin → D13 (self-cycling, just needs power)
void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
}
