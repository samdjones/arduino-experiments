// LCD1602 - RS→D7, E→D8, D4→D9, D5→D10, D6→D11, D7→D12
// VO → potentiometer wiper for contrast; VSS→GND, VDD→5V, RW→GND, A→5V(+resistor), K→GND
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  lcd.begin(16, 2);
  lcd.print("hello, world!");
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);
}
