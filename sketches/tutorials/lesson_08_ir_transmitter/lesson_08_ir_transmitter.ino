// IR transmitter - IR LED on D3 (IRremote default send pin)
// Requires a second Arduino running lesson_08_ir_receiver
#include <IRremote.h>

IRsend irsend;

void setup() {
  Serial.begin(9600);
  Serial.println("IR transmitter ready");
}

void loop() {
  Serial.println("Sending Sony 0xa90");
  irsend.sendSony(0xa90, 12);
  delay(40);
  irsend.sendSony(0xa90, 12);
  delay(40);
  irsend.sendSony(0xa90, 12);
  delay(2000);
}
