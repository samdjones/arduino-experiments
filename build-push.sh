#!/bin/bash

set -euo pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 <project_directory>"
  exit 1
fi

project_dir="$1"
if [ ! -d "$project_dir" ]; then
  echo "ERROR: Directory '$project_dir' does not exist."
  exit 1
fi

cd "$project_dir"

# Find single INO file in current directory
ino_count=$(ls *.ino 2>/dev/null | wc -l)
if [ "$ino_count" -eq 0 ]; then
  echo "ERROR: No .ino file found in directory '$project_dir'."
  exit 1
fi
if [ "$ino_count" -gt 1 ]; then
  echo "ERROR: Multiple .ino files found in directory '$project_dir'; expected exactly one."
  ls *.ino
  exit 1
fi
ino_file=$(ls *.ino)

# Discover board info from arduino-cli
if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "ERROR: arduino-cli not found in PATH."
  exit 1
fi
if ! command -v jq >/dev/null 2>&1; then
  echo "ERROR: jq not found in PATH. required for board parsing."
  exit 1
fi

board_json=$(arduino-cli board list --format json 2>/dev/null || true)
if [ -z "$board_json" ] || [ "$board_json" = "[]" ]; then
  echo "ERROR: No connected Arduino board was detected by 'arduino-cli board list'."
  exit 1
fi

fqbn=$(printf '%s' "$board_json" | jq -r '
  if has("detected_ports") then
    .detected_ports[]
    | select(.matching_boards != null)
    | .matching_boards[0].fqbn // empty
  elif type == "array" then
    .[0].fqbn // empty
  else
    .fqbn // empty
  end
' | sed -n '1p')

port=$(printf '%s' "$board_json" | jq -r '
  if has("detected_ports") then
    .detected_ports[]
    | select(.matching_boards != null)
    | .port.address // empty
  elif type == "array" then
    .[0].address // empty
  else
    .address // empty
  end
' | sed -n '1p')

if [ -z "$fqbn" ] || [ -z "$port" ]; then
  echo "ERROR: Could not infer board fqbn/address from 'arduino-cli board list'."
  echo "Output was:"
  printf '%s
' "$board_json"
  exit 1
fi

echo "Detected board: $fqbn on port $port"

echo "Compiling $ino_file"
arduino-cli compile --verbose --fqbn "$fqbn" "$ino_file"

echo "Uploading $ino_file to $fqbn on $port"
arduino-cli upload --port "$port" --fqbn "$fqbn" "$ino_file"

