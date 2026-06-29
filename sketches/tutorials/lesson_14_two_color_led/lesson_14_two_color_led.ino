// Two-color LED (5mm) - red→D11, green→D10
int redpin = 11;
int yellowpin = 10;
int val;

void setup() {
  pinMode(redpin, OUTPUT);
  pinMode(yellowpin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  for (val = 255; val > 0; val--) {
    analogWrite(11, val);
    analogWrite(10, 255 - val);
    delay(15);
  }
  for (val = 0; val < 255; val++) {
    analogWrite(11, val);
    analogWrite(10, 255 - val);
    delay(15);
  }
  Serial.println(val, DEC);
}
