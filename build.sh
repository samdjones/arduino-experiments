#!/bin/bash

set -euo pipefail

usage() {
  echo "Usage: $0 <project_directory> [--upload] [--monitor]"
  echo "By default the script only compiles the sketch."
  echo "Pass --upload to also upload it to the connected board."
  echo "Pass --monitor to open the serial monitor after upload."
  echo "Flags can be combined: $0 echo --upload --monitor"
}

upload=false
monitor=false

if [ $# -lt 1 ]; then
  usage
  exit 1
fi

project_dir="$1"
if [ "$project_dir" = "--upload" ] || [ "$project_dir" = "--monitor" ]; then
  echo "ERROR: Missing project directory."
  usage
  exit 1
fi

# Parse remaining arguments
shift
while [ $# -gt 0 ]; do
  case "$1" in
    --upload)
      upload=true
      ;;
    --monitor)
      monitor=true
      ;;
    *)
      echo "ERROR: Unknown argument '$1'."
      usage
      exit 1
      ;;
  esac
  shift
done

if [ ! -d "$project_dir" ]; then
  echo "ERROR: Directory '$project_dir' does not exist."
  exit 1
fi

project_dir=$(cd "$project_dir" && pwd)
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
serial_baud=9600

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

library_args=()
while IFS= read -r lib_dir; do
  library_args+=(--library "$lib_dir")
done < <(find "$project_dir" -type f -name library.properties -print | while read -r prop_path; do dirname "$prop_path"; done | sort -u)

if [ ${#library_args[@]} -eq 0 ]; then
  library_args+=(--library "$project_dir")
fi

echo "Compiling $ino_file"
compile_cmd=(arduino-cli compile --verbose --fqbn "$fqbn")
compile_cmd+=("${library_args[@]}")
compile_cmd+=("$ino_file")
"${compile_cmd[@]}"

if [ "$upload" = true ]; then
  echo "Uploading $ino_file to $fqbn on $port"
  arduino-cli upload --port "$port" --fqbn "$fqbn" "$ino_file"
  
  if [ "$monitor" = true ]; then
    echo "Opening serial monitor on $port at $serial_baud baud..."
    arduino-cli monitor --port "$port" --config "$serial_baud"
  fi
else
  echo "Build complete; skipping upload. Re-run with --upload to flash the board."
fi

