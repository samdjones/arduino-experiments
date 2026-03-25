# ServoSequencer API Reference

Version 1.2 — Generic servo keyframe sequencer for Arduino.

---

## Installation

Copy the `ServoSequencer/` folder into your Arduino `libraries/` directory. Requires the built-in **Servo** library (included with the Arduino IDE). The number of servos is set by `SS_NUM_SERVOS` in `ServoSequencer.h` — change it to suit your project before compiling.

```cpp
#include <ServoSequencer.h>

ServoSequencer seq;
```

---

## Concepts

### Servos

The library manages `SS_NUM_SERVOS` servos, referred to by index `0` through `SS_NUM_SERVOS - 1`. The default is `3`, but you can change it to any value in `ServoSequencer.h` before compiling. Each servo has its own pin, mechanical limits, rest position, and amplitude scale. You configure these once before calling `begin()`.

### Sequences

A sequence is a named collection of frames. It has three sections:

```
[ start frame ]  →  [ loop frame ] [ loop frame ] ... [ loop frame ]  →  [ stop frame ]
   (optional,           (one or more, repeat forever until halt())          (optional,
    plays once)                                                              plays once)
```

- The **start frame** plays once when you call `start()`, then the loop begins.
- The **loop body** repeats indefinitely until `halt()` is called.
- The **stop frame** plays once after the loop finishes, then the sequencer goes idle.

Both the start and stop frames are optional. If omitted, the loop starts and stops immediately.

### Frames

Each frame holds target angles for all 3 servos and a time budget (`delta_ms`) — the time allowed to travel from the previous frame's positions to this frame's positions. Use `-1` for a servo position to hold that servo still during the frame.

### Global parameters

Speed, easing, and per-servo scale are all global and can be changed at any time, including while a sequence is playing. Changes take effect on the next frame interpolation.

---

## Servo Setup

### `configServo()`

```cpp
seq.configServo(uint8_t servoIndex, uint8_t pin,
                int16_t minAngle  = 0,
                int16_t maxAngle  = 180,
                int16_t restAngle = 90);
```

Call once per servo, before `begin()`.

| Parameter | Description |
|---|---|
| `servoIndex` | Slot number: `0`, `1`, or `2` |
| `pin` | Arduino digital pin connected to the servo signal wire |
| `minAngle` | Mechanical minimum in degrees — the library will never command below this |
| `maxAngle` | Mechanical maximum in degrees — the library will never command above this |
| `restAngle` | Neutral position used by `begin()` and `rest()`, and the centre point for servo scaling |

**Tip:** measure your actual mechanical limits for each servo as mounted in the robot before setting these. Wrong limits can stall a motor or strip gears.

```cpp
// Servo 0 on pin 2, can travel 30°–150°, rests at 90°
seq.configServo(0,  2,  30, 150, 90);

// Servo 1 on pin 3, more restricted travel, rests at 90°
seq.configServo(1,  3,  60, 120, 90);

// Servo 2 on pin 4, full range
seq.configServo(2,  4,   0, 180, 90);
```

### `begin()`

```cpp
seq.begin();
```

Attaches all configured servos and moves them to their `restAngle`. Call once at the end of `setup()`, after all `configServo()` and sequence builder calls.

### `rest()`

```cpp
seq.rest();
```

Immediately moves all servos to their configured `restAngle`. Does not affect playback state — call `stop()` first if a sequence is running.

### `attach()` / `detach()`

```cpp
seq.attach();
seq.detach();
```

Attach or detach all servos from their pins. Detaching releases the PWM signal and lets the servo go limp, which saves power when the robot is idle. Re-attach before playing a sequence.

---

## Direct Control

These bypass the sequencer entirely and are useful during hardware bring-up or for one-off movements.

### `setAngle()`

```cpp
seq.setAngle(uint8_t servoIndex, int16_t angle);
```

Moves a servo immediately to the given angle (clamped to its configured min/max). No interpolation.

### `getAngle()`

```cpp
int16_t angle = seq.getAngle(uint8_t servoIndex);
```

Returns the servo's last commanded angle in degrees.

---

## Global Parameters

All three parameters are independent and can be changed at any time.

### Speed

```cpp
seq.setSpeed(float speed);
float s = seq.getSpeed();
```

Scales the time budget of every frame. `1.0` = normal speed. `2.0` = twice as fast (each frame takes half as long). `0.5` = half speed. Minimum is `0.01`.

```cpp
seq.setSpeed(1.0f);   // normal
seq.setSpeed(2.5f);   // fast run
seq.setSpeed(0.4f);   // slow creep
```

### Easing

```cpp
seq.setEasing(SS_Easing mode);
SS_Easing e = seq.getEasing();
```

Controls the movement curve within each frame — how a servo accelerates and decelerates as it travels from one position to the next.

| Mode | Behaviour |
|---|---|
| `SS_LINEAR` | Constant speed throughout — mechanical feeling |
| `SS_EASE_IN` | Starts slow, accelerates into the target |
| `SS_EASE_OUT` | Starts fast, decelerates into the target — good for planting a foot |
| `SS_EASE_IN_OUT` | Slow start, fast middle, slow finish — natural for a complete swing |
| `SS_SINE` | Like `EASE_IN_OUT` but softer at the extremes — most organic feeling |

```cpp
seq.setEasing(SS_SINE);
```

### Per-servo scale

```cpp
seq.setServoScale(uint8_t servoIndex, float scale);
float sc = seq.getServoScale(uint8_t servoIndex);
```

Scales each servo's swing around its `restAngle`. The formula applied to every target angle before it is sent to the servo:

```
final = restAngle + (targetAngle - restAngle) * scale
```

So `1.0` is full amplitude as written in the sequence. `0.5` halves the swing — the servo moves half as far from rest. `2.0` doubles it (still clamped to min/max). Use this to tune the same sequence between a high-stepping slow walk and a flat-footed fast run without rewriting any frames.

```cpp
// Full amplitude — high steps
seq.setServoScale(0, 1.0f);
seq.setServoScale(1, 1.0f);
seq.setServoScale(2, 1.0f);

// Reduced amplitude — low fast shuffle
seq.setServoScale(0, 0.4f);
seq.setServoScale(1, 0.4f);
seq.setServoScale(2, 0.4f);
```

---

## Building Sequences

Sequences are built once, typically in `setup()` or a function called from `setup()`. The builder call order is fixed:

```
beginSequence()
  setStartFrame()   ← optional, must come first
  addFrame()        ← one or more, these form the loop body
  setStopFrame()    ← optional, must come last
endSequence()
```

### `beginSequence()`

```cpp
int8_t index = seq.beginSequence(const char* name);
```

Starts building a new sequence with the given name. Returns the index it will be stored at (for use with `start(index)` later), or `-1` if the sequence store is full (`SS_MAX_SEQUENCES = 16`).

### `setStartFrame()`

```cpp
bool ok = seq.setStartFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]);
```

Optional. Adds a one-shot frame that plays once when `start()` is called, before the loop begins. Useful for moving servos into a ready position from wherever they were.

Must be called before any `addFrame()` calls.

### `addFrame()`

```cpp
bool ok = seq.addFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]);
```

Adds one frame to the loop body. Call as many times as needed. The loop body is everything added between `setStartFrame()` and `setStopFrame()`.

`delta_ms` is the time in milliseconds to travel from the previous frame's positions to this frame's positions, at `speed = 1.0`. Set a servo's angle to `-1` to leave it unchanged during that frame.

Up to `SS_MAX_FRAMES = 64` frames total per sequence (including start and stop frames).

### `setStopFrame()`

```cpp
bool ok = seq.setStopFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]);
```

Optional. Adds a one-shot frame that plays once after `halt()` has been called and the loop body has finished its current iteration. Useful for settling servos back to a rest position cleanly.

Must be called after all `addFrame()` calls.

### `endSequence()`

```cpp
bool ok = seq.endSequence();
```

Finalises and stores the sequence. Returns `false` if no loop-body frames were added.

### Complete example

```cpp
void buildWalk() {
  int16_t f[SS_NUM_SERVOS];

  seq.beginSequence("walk");

  // Start frame: move to walking stance over 500ms
  f[0]=70; f[1]=80; f[2]=70;
  seq.setStartFrame(500, f);

  // Loop body: 5-frame walking cycle
  f[0]=60;  f[1]=90; f[2]=120; seq.addFrame(400, f);  // right lifts
  f[0]=60;  f[1]=90; f[2]=90;  seq.addFrame(300, f);  // right plants
  f[0]=120; f[1]=90; f[2]=60;  seq.addFrame(400, f);  // left lifts
  f[0]=90;  f[1]=90; f[2]=60;  seq.addFrame(300, f);  // left plants
  f[0]=90;  f[1]=90; f[2]=90;  seq.addFrame(200, f);  // neutral

  // Stop frame: return to rest over 600ms
  f[0]=90; f[1]=90; f[2]=90;
  seq.setStopFrame(600, f);

  seq.endSequence();
}
```

---

## Playback

### `start()` / `startByName()`

```cpp
bool ok = seq.start(uint8_t seqIndex);
bool ok = seq.startByName(const char* name);
```

Begins playback. If the sequence has a start frame, it plays once; then the loop body repeats indefinitely. Returns `false` if the index or name is not found.

```cpp
seq.startByName("walk");
```

### `halt()`

```cpp
seq.halt();
```

Requests a graceful stop. The sequencer finishes the current loop iteration, then plays the stop frame (if the sequence has one), then goes idle. The robot ends up in a controlled position.

Use this for normal stopping — the animation completes naturally.

### `stop()`

```cpp
seq.stop();
```

Immediate hard stop. The sequencer goes idle instantly, mid-frame, with no stop frame played. Servos hold wherever they are.

Use this for emergency stops or when you need to switch sequences instantly.

### `pause()` / `resume()`

```cpp
seq.pause();
seq.resume();
```

Freezes and resumes playback mid-frame. Servos hold position while paused.

---

## Playback State

### `isRunning()`

```cpp
bool running = seq.isRunning();
```

Returns `true` while in the `START` or `LOOP` phase. False during the stop frame and when idle. Use this to know whether the gecko is actively moving (as opposed to winding down).

### `isPlaying()`

```cpp
bool playing = seq.isPlaying();
```

Returns `true` whenever the sequencer is doing anything (any phase except IDLE) and is not paused.

### `isPaused()`

```cpp
bool paused = seq.isPaused();
```

### `phase()`

```cpp
SS_Phase p = seq.phase();
```

Returns the current playback phase:

| Value | Meaning |
|---|---|
| `SS_PHASE_IDLE` | Not running |
| `SS_PHASE_START` | Playing the start frame |
| `SS_PHASE_LOOP` | Looping the loop body |
| `SS_PHASE_STOP` | Playing the stop frame |

Useful for triggering events at phase transitions, for example cutting motor power exactly when `SS_PHASE_IDLE` is reached after a halt.

### `currentSequence()` / `currentFrame()`

```cpp
uint8_t seqIdx   = seq.currentSequence();
uint8_t frameIdx = seq.currentFrame();
```

Returns the index of the currently playing sequence and frame.

---

## Main Loop

### `update()`

```cpp
seq.update();
```

Must be called on every iteration of `loop()`. This is where interpolation and servo writes happen. Do not use `delay()` anywhere in your sketch — it will stall the sequencer and cause jerky movement.

```cpp
void loop() {
  seq.update();   // always first
  // ... rest of your code, no delay()
}
```

---

## Debug

### `printStatus()`

```cpp
seq.printStatus();           // prints to Serial
seq.printStatus(Serial1);    // or any Stream
```

Prints a status block showing the current phase, active sequence, frame index, loop range, speed, easing, and all servo states. Useful during hardware bring-up.

Example output:
```
-- ServoSequencer -------------------
  Sequences : 1
  Speed     : 1.00
  Easing    : SINE
  Phase     : LOOP
  Halting   : no
  Sequence  : walk
  Frame     : 2  (loop 1..5  start=0  stop=6)
  Servo[0]  pin=2  angle=87deg  scale=1.00
  Servo[1]  pin=3  angle=90deg  scale=1.00
  Servo[2]  pin=4  angle=103deg  scale=1.00
-------------------------------------
```

---

## Constants

All three constants live at the top of `ServoSequencer.h`. Change them before compiling to suit your project.

| Constant | Default | Description |
|---|---|---|
| `SS_NUM_SERVOS` | `3` | Number of servos. Change this to match your hardware. |
| `SS_MAX_FRAMES` | `64` | Max frames per sequence (including start/stop frames) |
| `SS_MAX_SEQUENCES` | `16` | Max number of stored sequences |

### Changing the servo count

`SS_NUM_SERVOS` is the primary configuration knob. Set it to however many servos your project uses:

```cpp
// In ServoSequencer.h:
#define SS_NUM_SERVOS     12   // e.g. a 12-servo quadruped
#define SS_NUM_SERVOS     6    // e.g. a hexapod leg
#define SS_NUM_SERVOS     2    // e.g. a simple pan/tilt
```

Everything in the library — frame arrays, servo config arrays, scale arrays — sizes itself from this one constant. The `angles[]` array you pass to `addFrame()`, `setStartFrame()`, and `setStopFrame()` must always have exactly `SS_NUM_SERVOS` elements.

### RAM usage

Each `SS_Sequence` in memory occupies approximately:

```
SS_MAX_FRAMES × (2 + SS_NUM_SERVOS × 2)  bytes
```

Multiplied by `SS_MAX_SEQUENCES` for the total sequence store. On AVR boards (Uno, Nano) with only 2 KB of RAM, keep `SS_NUM_SERVOS`, `SS_MAX_FRAMES`, and `SS_MAX_SEQUENCES` as small as your project allows. On ESP32 or ARM-based boards RAM is rarely a concern.

---

## Minimal sketch template

```cpp
#include <ServoSequencer.h>

ServoSequencer seq;

void setup() {
  seq.configServo(0, 2,  30, 150, 90);
  seq.configServo(1, 3,  45, 135, 90);
  seq.configServo(2, 4,  30, 150, 90);

  int16_t f[SS_NUM_SERVOS];
  seq.beginSequence("mySeq");
  f[0]=60; f[1]=90; f[2]=120; seq.addFrame(400, f);
  f[0]=90; f[1]=90; f[2]=90;  seq.addFrame(400, f);
  seq.endSequence();

  seq.begin();
  seq.setEasing(SS_SINE);
  seq.startByName("mySeq");
}

void loop() {
  seq.update();
}
```
