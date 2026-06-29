#include <Arduino.h>

// constants won't change. They're used here to set pin numbers:
const int BUTTON = 2;
const int LED = LED_BUILTIN; 

// variables will change:
bool runState = false;
const long DEBOUNCE_DELAY = 50; // debounce delay in milliseconds
unsigned long lastDebounceTime = millis(); // the last time the button state was toggled
bool lastButtonState = false;
int lastRawButtonState = LOW;
unsigned long eventCounter = 0;

// setup() runs once when you press reset or power the board
void  setup()
{
  Serial.begin(9600); // Set to 9600 for Serial Monitor matching
  while (!Serial) {
    ;
  }
  pinMode(BUTTON, INPUT);
  pinMode(LED, OUTPUT);
  lastRawButtonState = digitalRead(BUTTON);
}

// loop() runs over and over again forever
void  loop()
{
  int rawButtonState = digitalRead(BUTTON);
  if (rawButtonState != lastRawButtonState) {
    lastRawButtonState = rawButtonState;
    ++eventCounter;
    Serial.print(eventCounter);
    Serial.print(": ");
    if (rawButtonState == HIGH) {
      for (int i = 0; i < 10; ++i) Serial.print('+');
      Serial.println();
    } else {
      for (int i = 0; i < 10; ++i) Serial.print('-');
      Serial.println();
    }
  }

  if ((millis() - lastDebounceTime) >= DEBOUNCE_DELAY) {
    int buttonState = rawButtonState;
    if (buttonState != lastButtonState)
    {
      lastButtonState = buttonState;
      lastDebounceTime = millis();
      if (buttonState == HIGH) {
        ++eventCounter;
        Serial.print(eventCounter);
        Serial.print(": ");
        runState = !runState;
        for (int i = 0; i < 10; ++i) Serial.print('*');
        Serial.println();
      }
    }
  }
  digitalWrite(LED,  runState);
}