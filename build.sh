#!/bin/bash

set -euo pipefail

usage() {
  echo "Usage: $0 <project_directory> [--fqbn <board>] [--upload] [--monitor]"
  echo "By default the script only compiles the sketch."
  echo "Pass --fqbn to compile without a connected board."
  echo "Pass --upload to also upload it to the connected board."
  echo "Pass --monitor to open the serial monitor after upload."
  echo "Flags can be combined: $0 echo --fqbn arduino:avr:leonardo --upload --monitor"
}

upload=false
monitor=false
fqbn=""
project_dir=""

if [ $# -lt 1 ]; then
  usage
  exit 1
fi

while [ $# -gt 0 ]; do
  case "$1" in
    --upload)
      upload=true
      ;;
    --monitor)
      monitor=true
      ;;
    --fqbn)
      shift
      if [ $# -eq 0 ] || [ "${1#-}" != "$1" ]; then
        echo "ERROR: --fqbn requires a board name argument."
        usage
        exit 1
      fi
      fqbn="$1"
      ;;
    --*)
      echo "ERROR: Unknown option '$1'."
      usage
      exit 1
      ;;
    *)
      if [ -z "$project_dir" ]; then
        project_dir="$1"
      else
        echo "ERROR: Multiple project directories provided: '$project_dir' and '$1'."
        usage
        exit 1
      fi
      ;;
  esac
  shift
done

if [ -z "$project_dir" ]; then
  echo "ERROR: Missing project directory."
  usage
  exit 1
fi

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
  if [ -z "$fqbn" ]; then
    if [ "$upload" = true ] || [ "$monitor" = true ]; then
      echo "ERROR: No connected Arduino board was detected and --fqbn was not provided."
      exit 1
    fi
    echo "ERROR: No connected Arduino board was detected. Use --fqbn to compile without a board."
    exit 1
  fi
fi

if [ -z "$fqbn" ]; then
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
fi

port=""
if [ "$upload" = true ] || [ "$monitor" = true ]; then
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
fi

if [ "$upload" = true ] || [ "$monitor" = true ]; then
  echo "Detected board: $fqbn on port $port"
else
  echo "Using FQBN: $fqbn"
fi

declare -a library_args=()
while IFS= read -r lib_dir; do
  library_args+=(--library "$lib_dir")
done < <(find "$project_dir" -type f -name library.properties -print | while read -r prop_path; do dirname "$prop_path"; done | sort -u)

echo "Compiling $ino_file"
compile_cmd=(arduino-cli compile --verbose --fqbn "$fqbn")
if [ "${#library_args[@]:-0}" -gt 0 ]; then
  compile_cmd+=("${library_args[@]}")
fi
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

