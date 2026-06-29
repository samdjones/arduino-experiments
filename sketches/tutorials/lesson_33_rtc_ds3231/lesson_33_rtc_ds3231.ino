// DS3231 RTC - SCL→A5(SCL), SDA→A4(SDA), VCC→5V, GND
// Uses NorthernWidget DS3231 library (getSecond/getMinute/etc. API)
#include <Wire.h>
#include <DS3231.h>

DS3231 rtc;
bool century = false;
bool h12Flag;
bool pmFlag;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("DS3231 RTC ready");
  // To set the time, uncomment and edit these lines, upload once, then comment out again:
  // rtc.setYear(25); rtc.setMonth(1); rtc.setDate(1);
  // rtc.setHour(12); rtc.setMinute(0); rtc.setSecond(0);
  // rtc.setDoW(4); // 1=Sun, 2=Mon, ..., 7=Sat
}

void loop() {
  int second = rtc.getSecond();
  int minute = rtc.getMinute();
  int hour   = rtc.getHour(h12Flag, pmFlag);
  int date   = rtc.getDate();
  int month  = rtc.getMonth(century);
  int year   = rtc.getYear();

  Serial.print("20");
  Serial.print(year, DEC);   Serial.print("-");
  if (month  < 10) Serial.print("0");
  Serial.print(month, DEC);  Serial.print("-");
  if (date   < 10) Serial.print("0");
  Serial.print(date, DEC);   Serial.print(" ");
  Serial.print(hour, DEC);   Serial.print(":");
  if (minute < 10) Serial.print("0");
  Serial.print(minute, DEC); Serial.print(":");
  if (second < 10) Serial.print("0");
  Serial.println(second, DEC);
  delay(1000);
}
