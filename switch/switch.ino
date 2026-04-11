#include <Arduino.h>

// constants won't change. They're used here to set pin numbers:
const int BUTTON = 2;
const int LED = LED_BUILTIN; 

// variables will change:
bool runState = false;
const long DEBOUNCE_DELAY = 50; // debounce delay in milliseconds
unsigned long lastDebounceTime = millis(); // the last time the button state was toggled
bool lastButtonState = false;

// setup() runs once when you press reset or power the board
void  setup()
{
  Serial.begin(9600);
  pinMode(BUTTON, INPUT);
  pinMode(LED, OUTPUT);
}

// loop() runs over and over again forever
void  loop()
{
  if ((millis() - lastDebounceTime) >= DEBOUNCE_DELAY) {
    int buttonState = digitalRead(BUTTON);
    if (buttonState != lastButtonState)
    {
      lastButtonState = buttonState;
      lastDebounceTime = millis();
      if (buttonState == HIGH) runState = !runState;
    }
  }
  digitalWrite(LED,  runState);
}