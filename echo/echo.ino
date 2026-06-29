#include <Arduino.h>

/*
  Arduino-CLI quick usage (how to connect / compile / upload / monitor):

  1) List connected boards and ports (to find the port and fqbn):
    arduino-cli board list --format json
    arduino-cli board list

  2) Compile the sketch (from the sketch folder or repo root):
    arduino-cli compile --fqbn <fqbn> --libraries /path/to/projectDir echo.ino
    # or use the helper script from the repo root:
    ./build.sh echo

  3) Upload to the detected board (if you want to flash):
    arduino-cli upload --port /dev/tty.usbmodemXXXX --fqbn <fqbn> echo.ino
    # or with helper script:
    ./build.sh echo --upload

  4) Open a serial monitor to interact:
    arduino-cli monitor --port /dev/cu.usbmodem14101
    # or use python miniterm if pyserial is installed:
    python -m serial.tools.miniterm /dev/tty.usbmodemXXXX 9600

  Notes:
  - The device path on macOS usually looks like /dev/tty.usbmodemXXXX or /dev/tty.usbserial-XXXX.
*/

const size_t BUFFER_SIZE = 64;
char inputBuffer[BUFFER_SIZE];

void setup() {
  Serial.begin(9600);
  // Some boards require waiting for the USB serial connection
  // (Leonardo, Micro). On UNO this returns immediately.
  unsigned long start = millis();
  while (!Serial && millis() - start < 2000) {
    ;
  }
  Serial.println("Enter a string and press Enter to echo it back.");
}

void loop() {
  static size_t index = 0;

  while (Serial.available() > 0) {
    int r = Serial.read();
    if (r < 0) break;
    uint8_t c = (uint8_t)r;

    // Treat CR or LF as end-of-line
    if (c == '\n' || c == '\r') {
      if (index > 0) {
        inputBuffer[index] = '\0';
        Serial.print("Echo: ");
        Serial.println(inputBuffer);
        index = 0;
      }
    } else {
      if (index < BUFFER_SIZE - 1) {
        inputBuffer[index++] = (char)c;
      }
    }
  }
}
