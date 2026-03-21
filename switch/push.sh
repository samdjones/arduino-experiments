#!/bin/bash

set -euo pipefail

cd /Users/sam/code/plan909/arduino-experiments
arduino-cli compile --verbose --fqbn arduino:avr:leonardo switch/switch.ino
arduino-cli upload --port /dev/cu.usbmodem14201 --fqbn arduino:avr:leonardo switch/switch.ino
