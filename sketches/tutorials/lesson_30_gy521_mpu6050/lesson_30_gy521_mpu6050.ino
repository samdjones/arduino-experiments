// GY-521 (MPU-6050) - SDA→A4, SCL→A5 (I2C), VCC→3.3V or 5V, GND
#include <Wire.h>

const int MPU_ADDR = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // wake up the MPU-6050
  Wire.endTransmission(true);
  Serial.begin(9600);
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
  Serial.print("AcX="); Serial.print(AcX);
  Serial.print(" AcY="); Serial.print(AcY);
  Serial.print(" AcZ="); Serial.print(AcZ);
  Serial.print(" Tmp="); Serial.print(Tmp / 340.00 + 36.53);
  Serial.print(" GyX="); Serial.print(GyX);
  Serial.print(" GyY="); Serial.print(GyY);
  Serial.print(" GyZ="); Serial.println(GyZ);
  delay(333);
}
