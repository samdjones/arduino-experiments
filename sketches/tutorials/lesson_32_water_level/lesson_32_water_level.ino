// Water level sensor - S pin → A0
int adc_id = 0;
int HistoryValue = 0;
char printBuffer[128];

void setup() {
  Serial.begin(9600);
}

void loop() {
  int currentValue = analogRead(adc_id);
  if (abs(currentValue - HistoryValue) > 10) {
    sprintf(printBuffer, "ADC%d water level: %d", adc_id, currentValue);
    Serial.println(printBuffer);
    HistoryValue = currentValue;
  }
}
