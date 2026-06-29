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
./build.sh sketches/<project_directory>
```

The script will:

- verify that the target directory exists
- require exactly one `.ino` file in that directory
- detect the connected board and serial port with `arduino-cli`
- compile the sketch

To upload the compiled sketch to the board, add `--upload`:

```bash
./build.sh sketches/<project_directory> --upload
```

To upload and then open the Arduino CLI serial monitor automatically, add `--monitor`:

```bash
./build.sh sketches/<project_directory> --upload --monitor
```

To compile without a connected board, pass `--fqbn` with the fully qualified board name:

```bash
./build.sh sketches/<project_directory> --fqbn arduino:avr:leonardo
```

### Examples

```bash
./build.sh sketches/blink
./build.sh sketches/servo --upload
./build.sh sketches/robogekko --upload --monitor
./build.sh sketches/echo --fqbn arduino:avr:leonardo
```

If a project directory contains multiple `.ino` files, the script will stop and ask you to simplify it to a single sketch.

-