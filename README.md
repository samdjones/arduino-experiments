# arduino-experiments

A small collection of Arduino sketches and experiments.

## Build and upload a sketch

Use the helper script at the repository root to compile and upload a sketch from one of the project directories.

### Prerequisites

- Arduino CLI installed and available in your PATH
- `jq` installed for parsing board information
- An Arduino board connected and detected by `arduino-cli board list`

### Usage

From the repository root, run:

```bash
./build-push.sh <project_directory>
```

The script will:

- verify that the target directory exists
- require exactly one `.ino` file in that directory
- detect the connected board and serial port with `arduino-cli`
- compile the sketch
- upload it to the detected board

### Examples

```bash
./build-push.sh blink
./build-push.sh servo
./build-push.sh robogekko
```

If a project directory contains multiple `.ino` files, the script will stop and ask you to simplify it to a single sketch.

