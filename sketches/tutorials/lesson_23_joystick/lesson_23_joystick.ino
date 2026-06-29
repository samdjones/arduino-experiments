// Joystick - SW→D2, X→A1, Y→A2, VCC, GND
const int SW_pin = 2;
const int X_pin = A1;
const int Y_pin = A2;

void setup() {
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  Serial.begin(9600);
}

void loop() {
  Serial.print("SW: ");
  Serial.print(digitalRead(SW_pin));
  Serial.print(" | X: ");
  Serial.print(analogRead(X_pin));
  Serial.print(" | Y: ");
  Serial.println(analogRead(Y_pin));
  delay(500);
}
