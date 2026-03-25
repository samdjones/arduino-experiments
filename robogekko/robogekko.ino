// =============================================================
//  ServoSequencer_Example.ino  v1.2
//
//  Demonstrates start frame, loop body, and stop frame.
//
//  "walk" sequence:
//    start frame : servos rise from rest to walking stance
//    loop  frames: the actual walking cycle
//    stop  frame : servos settle back to rest
//
//  Serial commands:
//    g = start("walk")   h = halt (graceful, plays stop frame)
//    s = stop (immediate, no stop frame)
//    p = pause / resume
//    + / -  speed        e = cycle easing
//    i = print status
// =============================================================

#include <ServoSequencer.h>

ServoSequencer seq;

void buildWalk() {
  int16_t f[SS_NUM_SERVOS];

  seq.beginSequence("walk");

  // -- Start frame: rise to walking stance (played once) ------
  f[0]=70; f[1]=80; f[2]=70;
  seq.setStartFrame(500, f);

  // -- Loop body: walking cycle --------------------------------
  f[0]=60;  f[1]=90; f[2]=120; seq.addFrame(400, f);  // right lifts
  f[0]=60;  f[1]=90; f[2]=90;  seq.addFrame(300, f);  // right plants
  f[0]=120; f[1]=90; f[2]=60;  seq.addFrame(400, f);  // left lifts
  f[0]=90;  f[1]=90; f[2]=60;  seq.addFrame(300, f);  // left plants
  f[0]=90;  f[1]=90; f[2]=90;  seq.addFrame(200, f);  // neutral

  // -- Stop frame: settle back to rest (played once) ----------
  f[0]=90; f[1]=90; f[2]=90;
  seq.setStopFrame(600, f);

  seq.endSequence();
}

void setup() {
  Serial.begin(115200);

  seq.configServo(0,  2,  30, 150, 90);
  seq.configServo(1,  3,  45, 135, 90);
  seq.configServo(2,  4,  30, 150, 90);

  buildWalk();
  seq.begin();
 
        seq.setEasing(SS_SINE);
        seq.setSpeed(1.0f);
        seq.startByName("walk");
        Serial.println(F("-> walk started"));


  Serial.println(F("ServoSequencer v1.2 ready."));
  Serial.println(F("  g = start walk   h = halt (graceful)"));
  Serial.println(F("  s = stop (hard)  p = pause/resume"));
  Serial.println(F("  + / -  speed     e = cycle easing"));
  Serial.println(F("  i = status"));
}

void loop() {
  seq.update();

  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {

      case 'g':
        seq.setEasing(SS_SINE);
        seq.setSpeed(1.0f);
        seq.startByName("walk");
        Serial.println(F("-> walk started"));
        break;

      case 'h':
        seq.halt();
        Serial.println(F("-> halt requested (will finish loop + play stop frame)"));
        break;

      case 's':
        seq.stop();
        Serial.println(F("-> stopped immediately"));
        break;

      case 'p':
        if (seq.isPaused()) { seq.resume(); Serial.println(F("-> resumed")); }
        else                { seq.pause();  Serial.println(F("-> paused"));  }
        break;

      case '+':
        seq.setSpeed(seq.getSpeed() * 1.25f);
        Serial.print(F("speed -> ")); Serial.println(seq.getSpeed());
        break;

      case '-':
        seq.setSpeed(seq.getSpeed() * 0.8f);
        Serial.print(F("speed -> ")); Serial.println(seq.getSpeed());
        break;

      case 'e': {
        SS_Easing next = (SS_Easing)((seq.getEasing() + 1) % 5);
        seq.setEasing(next);
        const char* names[] = {"LINEAR","EASE_IN","EASE_OUT","EASE_IN_OUT","SINE"};
        Serial.print(F("easing -> ")); Serial.println(names[next]);
        break;
      }

      case 'i':
        seq.printStatus(Serial);
        break;
    }
  }
}
