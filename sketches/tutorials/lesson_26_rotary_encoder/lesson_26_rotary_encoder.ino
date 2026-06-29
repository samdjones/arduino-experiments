// Rotary encoder - CLK→D2, DT→D3, SW→D4
int CLK = 2;
int DT = 3;
int SW = 4;
const int interrupt0 = 0;
int count = 0;
int lastCLK = 0;

void ClockChanged() {
  int clkValue = digitalRead(CLK);
  int dtValue = digitalRead(DT);
  if (lastCLK != clkValue) {
    lastCLK = clkValue;
    count += (clkValue != dtValue) ? 1 : -1;
  }
}

void setup() {
  pinMode(SW, INPUT);
  digitalWrite(SW, HIGH);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  attachInterrupt(interrupt0, ClockChanged, CHANGE);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(SW) == LOW) {
    count = 0;
    Serial.println("Count reset");
    delay(200);
  }
  Serial.println(count);
  delay(100);
}
