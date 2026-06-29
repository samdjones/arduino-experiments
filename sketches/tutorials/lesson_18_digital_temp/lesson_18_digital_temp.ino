// NTC thermistor module - AO→A0, DO→D3
// Steinhart-Hart equation for 10k NTC thermistor
#include <math.h>

const int sensorPin = A0;
const int digitalPin = 3;

double Thermistor(int RawADC) {
  double Temp = log(10000.0 * ((1024.0 / RawADC) - 1.0));
  Temp = 1.0 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
  return Temp - 273.15;
}

void setup() {
  pinMode(digitalPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  int rawValue = analogRead(sensorPin);
  double temp = Thermistor(rawValue);
  Serial.print("Temperature: ");
  Serial.print(temp, 1);
  Serial.print(" C | Digital: ");
  Serial.println(digitalRead(digitalPin));
  delay(1000);
}
