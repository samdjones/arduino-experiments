#ifndef SERVO_SEQUENCER_H
#define SERVO_SEQUENCER_H

#include <Arduino.h>
#include <Servo.h>

// -------------------------------------------------------------
//  ServoSequencer  v1.2
//  Generic multi-servo keyframe sequencer
// -------------------------------------------------------------

#define SS_NUM_SERVOS     3
#define SS_MAX_FRAMES     64
#define SS_MAX_SEQUENCES  16

// Global easing mode - applies to all frames, changeable anytime
enum SS_Easing {
  SS_LINEAR,
  SS_EASE_IN,
  SS_EASE_OUT,
  SS_EASE_IN_OUT,
  SS_SINE
};

// Internal playback phase
enum SS_Phase {
  SS_PHASE_IDLE,    // not running
  SS_PHASE_START,   // playing start frame (once)
  SS_PHASE_LOOP,    // looping between loopFirst..loopLast
  SS_PHASE_STOP     // playing stop frame (once), then idle
};

// -- Frame -----------------------------------------------------
// pos[i] is the target angle for servo i.
// delta_ms is the time from the PREVIOUS frame to reach this one.
struct SS_Frame {
  uint16_t  delta_ms;
  int16_t   pos[SS_NUM_SERVOS];  // -1 = hold this servo
};

// -- Sequence --------------------------------------------------
// Frame layout inside the frames[] array:
//
//   index 0              : start frame  (if startFrame >= 0)
//   loopFirst..loopLast  : loop body    (always present)
//   count-1              : stop frame   (if stopFrame  >= 0)
//
// These indices are managed automatically by the builder.
struct SS_Sequence {
  const char* name;
  SS_Frame    frames[SS_MAX_FRAMES];
  uint8_t     count;
  int8_t      startFrame;   // frame index of start frame, or -1
  int8_t      stopFrame;    // frame index of stop  frame, or -1
  uint8_t     loopFirst;    // first frame of loop body
  uint8_t     loopLast;     // last  frame of loop body
};

// -------------------------------------------------------------

class ServoSequencer {
public:
  ServoSequencer();

  // -- Hardware setup ------------------------------------------
  void  configServo(uint8_t servoIndex, uint8_t pin,
                    int16_t minAngle  = 0,
                    int16_t maxAngle  = 180,
                    int16_t restAngle = 90);

  void  begin();
  void  rest();
  void  detach();
  void  attach();

  // -- Direct control ------------------------------------------
  void    setAngle(uint8_t servoIndex, int16_t angle);
  int16_t getAngle(uint8_t servoIndex) const;

  // -- Multipliers ---------------------------------------------
  void      setSpeed(float speed);
  float     getSpeed() const;

  void      setEasing(SS_Easing easing);
  SS_Easing getEasing() const;

  void  setServoScale(uint8_t servoIndex, float scale);
  float getServoScale(uint8_t servoIndex) const;

  // -- Sequence builder ----------------------------------------
  // Returns sequence index (or -1 on failure).
  // After beginSequence(), call:
  //   setStartFrame()  - optional, adds a one-shot lead-in frame
  //   addFrame()       - one or more loop body frames
  //   setStopFrame()   - optional, adds a one-shot wind-down frame
  //   endSequence()
  int8_t beginSequence(const char* name);
  bool   setStartFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]);
  bool   addFrame     (uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]);
  bool   setStopFrame (uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]);
  bool   endSequence();

  // -- Playback ------------------------------------------------
  // start() : play start frame (if any), then loop continuously.
  // halt()  : finish current frame, play stop frame (if any), then idle.
  // stop()  : immediate hard stop, no stop frame.
  bool  start(uint8_t seqIndex);
  bool  startByName(const char* name);
  void  halt();   // graceful: finishes loop iteration, plays stop frame
  void  stop();   // immediate

  void  pause();
  void  resume();

  bool    isRunning() const;   // true while in START or LOOP phase
  bool    isPlaying() const;   // true while not IDLE and not paused
  bool    isPaused()  const;
  SS_Phase phase()    const;
  uint8_t  currentSequence() const;
  uint8_t  currentFrame()    const;

  // -- Main loop -----------------------------------------------
  void  update();

  // -- Debug ---------------------------------------------------
  void  printStatus(Stream& s = Serial);

private:
  // Hardware
  Servo   _servo[SS_NUM_SERVOS];
  uint8_t _pin[SS_NUM_SERVOS];
  int16_t _minAngle[SS_NUM_SERVOS];
  int16_t _maxAngle[SS_NUM_SERVOS];
  int16_t _restAngle[SS_NUM_SERVOS];
  int16_t _currentAngle[SS_NUM_SERVOS];
  float   _scale[SS_NUM_SERVOS];

  // Sequences
  SS_Sequence _seq[SS_MAX_SEQUENCES];
  uint8_t     _seqCount;

  // Build buffer
  SS_Sequence _build;
  bool        _building;

  // Playback
  SS_Phase      _phase;
  bool          _paused;
  bool          _haltPending;   // halt() was called; stop after loop end
  uint8_t       _playIdx;
  uint8_t       _frameIdx;
  unsigned long _frameStart;
  int16_t       _fromAngle[SS_NUM_SERVOS];
  float         _speed;
  SS_Easing     _easing;

  // Helpers
  void    _startFrame(uint8_t frameIdx);
  void    _writeServo(uint8_t i, int16_t angle);
  int16_t _clamp(uint8_t i, int16_t angle) const;
  int16_t _applyScale(uint8_t i, int16_t target) const;
  float   _ease(float t) const;
  int8_t  _findByName(const char* name) const;
  void    _snapshotFrom();
};

#endif // SERVO_SEQUENCER_H
