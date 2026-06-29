// RGB LED module - R→D11, G→D10, B→D9
int redpin = 11;
int greenpin = 10;
int bluepin = 9;
int val;

void setup() {
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  for (val = 255; val > 0; val--) {
    analogWrite(11, val);
    analogWrite(10, 255 - val);
    analogWrite(9, 128 - val);
    delay(50);
  }
  for (val = 0; val < 255; val++) {
    analogWrite(11, val);
    analogWrite(10, 255 - val);
    analogWrite(9, 128 - val);
    delay(50);
  }
  Serial.println(val, DEC);
}
