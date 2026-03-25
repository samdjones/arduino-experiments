#include "ServoSequencer.h"
#include <string.h>
#include <math.h>

// -------------------------------------------------------------
//  ServoSequencer.cpp  v1.2
// -------------------------------------------------------------

ServoSequencer::ServoSequencer() {
  _seqCount    = 0;
  _building    = false;
  _phase       = SS_PHASE_IDLE;
  _paused      = false;
  _haltPending = false;
  _playIdx     = 0;
  _frameIdx    = 0;
  _frameStart  = 0;
  _speed       = 1.0f;
  _easing      = SS_LINEAR;

  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) {
    _pin[i]          = 255;
    _minAngle[i]     = 0;
    _maxAngle[i]     = 180;
    _restAngle[i]    = 90;
    _currentAngle[i] = 90;
    _fromAngle[i]    = 90;
    _scale[i]        = 1.0f;
  }
}

// -- Hardware setup --------------------------------------------

void ServoSequencer::configServo(uint8_t i, uint8_t pin,
                                  int16_t minAngle, int16_t maxAngle,
                                  int16_t restAngle) {
  if (i >= SS_NUM_SERVOS) return;
  _pin[i]       = pin;
  _minAngle[i]  = minAngle;
  _maxAngle[i]  = maxAngle;
  _restAngle[i] = restAngle;
}

void ServoSequencer::begin() {
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) {
    if (_pin[i] == 255) continue;
    _servo[i].attach(_pin[i]);
    _writeServo(i, _restAngle[i]);
  }
}

void ServoSequencer::rest() {
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++)
    _writeServo(i, _restAngle[i]);
}

void ServoSequencer::detach() {
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++)
    _servo[i].detach();
}

void ServoSequencer::attach() {
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++)
    if (_pin[i] != 255) _servo[i].attach(_pin[i]);
}

// -- Direct control --------------------------------------------

void ServoSequencer::setAngle(uint8_t i, int16_t angle) {
  if (i >= SS_NUM_SERVOS) return;
  _writeServo(i, angle);
}

int16_t ServoSequencer::getAngle(uint8_t i) const {
  if (i >= SS_NUM_SERVOS) return -1;
  return _currentAngle[i];
}

// -- Multipliers -----------------------------------------------

void      ServoSequencer::setSpeed(float s)          { _speed  = (s < 0.01f) ? 0.01f : s; }
float     ServoSequencer::getSpeed()           const  { return _speed; }
void      ServoSequencer::setEasing(SS_Easing e)      { _easing = e; }
SS_Easing ServoSequencer::getEasing()          const  { return _easing; }

void  ServoSequencer::setServoScale(uint8_t i, float sc) {
  if (i < SS_NUM_SERVOS) _scale[i] = sc;
}
float ServoSequencer::getServoScale(uint8_t i) const {
  return (i < SS_NUM_SERVOS) ? _scale[i] : 1.0f;
}

// -- Sequence builder ------------------------------------------

int8_t ServoSequencer::beginSequence(const char* name) {
  if (_building)                     return -1;
  if (_seqCount >= SS_MAX_SEQUENCES) return -1;

  memset(&_build, 0, sizeof(SS_Sequence));
  _build.name       = name;
  _build.count      = 0;
  _build.startFrame = -1;
  _build.stopFrame  = -1;
  _build.loopFirst  = 0;
  _build.loopLast   = 0;
  _building         = true;
  return (int8_t)_seqCount;
}

bool ServoSequencer::setStartFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]) {
  if (!_building)                    return false;
  if (_build.count > 0)              return false; // must be first
  if (_build.count >= SS_MAX_FRAMES) return false;

  SS_Frame& f = _build.frames[_build.count];
  f.delta_ms  = delta_ms;
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) f.pos[i] = angles[i];

  _build.startFrame = (int8_t)_build.count;
  _build.count++;
  _build.loopFirst = _build.count;  // loop starts after this
  return true;
}

bool ServoSequencer::addFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]) {
  if (!_building)                    return false;
  if (_build.count >= SS_MAX_FRAMES) return false;

  SS_Frame& f = _build.frames[_build.count];
  f.delta_ms  = delta_ms;
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) f.pos[i] = angles[i];

  _build.loopLast = _build.count;   // keep updating; last addFrame wins
  _build.count++;
  return true;
}

bool ServoSequencer::setStopFrame(uint16_t delta_ms, int16_t angles[SS_NUM_SERVOS]) {
  if (!_building)                    return false;
  if (_build.count == 0)             return false; // need at least one loop frame
  if (_build.count >= SS_MAX_FRAMES) return false;

  SS_Frame& f = _build.frames[_build.count];
  f.delta_ms  = delta_ms;
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) f.pos[i] = angles[i];

  _build.stopFrame = (int8_t)_build.count;
  _build.count++;
  return true;
}

bool ServoSequencer::endSequence() {
  // Must have at least one loop-body frame
  uint8_t loopFrames = _build.loopLast - _build.loopFirst + 1;
  if (!_building || loopFrames == 0) return false;

  memcpy(&_seq[_seqCount++], &_build, sizeof(SS_Sequence));
  _building = false;
  return true;
}

// -- Playback --------------------------------------------------

bool ServoSequencer::start(uint8_t idx) {
  if (idx >= _seqCount) return false;

  _playIdx     = idx;
  _paused      = false;
  _haltPending = false;

  SS_Sequence& s = _seq[idx];

  if (s.startFrame >= 0) {
    _phase = SS_PHASE_START;
    _startFrame((uint8_t)s.startFrame);
  } else {
    _phase = SS_PHASE_LOOP;
    _startFrame(s.loopFirst);
  }
  return true;
}

bool ServoSequencer::startByName(const char* name) {
  int8_t idx = _findByName(name);
  return (idx >= 0) ? start((uint8_t)idx) : false;
}

// Graceful stop: finish the current loop iteration, play stop frame, then idle.
void ServoSequencer::halt() {
  if (_phase == SS_PHASE_LOOP)
    _haltPending = true;
  else if (_phase == SS_PHASE_START)
    _haltPending = true;   // will catch it when loop would begin
}

// Immediate stop.
void ServoSequencer::stop() {
  _phase       = SS_PHASE_IDLE;
  _paused      = false;
  _haltPending = false;
}

void ServoSequencer::pause()  { if (_phase != SS_PHASE_IDLE && !_paused) _paused = true; }
void ServoSequencer::resume() { if (_paused) { _paused = false; _frameStart = millis(); } }

bool     ServoSequencer::isRunning() const { return _phase == SS_PHASE_START || _phase == SS_PHASE_LOOP; }
bool     ServoSequencer::isPlaying() const { return _phase != SS_PHASE_IDLE && !_paused; }
bool     ServoSequencer::isPaused()  const { return _paused; }
SS_Phase ServoSequencer::phase()     const { return _phase; }
uint8_t  ServoSequencer::currentSequence() const { return _playIdx; }
uint8_t  ServoSequencer::currentFrame()    const { return _frameIdx; }

// -- Update ----------------------------------------------------

void ServoSequencer::update() {
  if (_phase == SS_PHASE_IDLE || _paused) return;

  SS_Sequence& seq = _seq[_playIdx];
  SS_Frame&    f   = seq.frames[_frameIdx];

  uint16_t      dur     = (uint16_t)((float)f.delta_ms / _speed);
  unsigned long elapsed = millis() - _frameStart;
  float t  = (dur > 0) ? ((float)elapsed / (float)dur) : 1.0f;
  if (t > 1.0f) t = 1.0f;
  float te = _ease(t);

  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) {
    if (f.pos[i] < 0) continue;
    int16_t target  = _applyScale(i, f.pos[i]);
    int16_t current = (int16_t)(_fromAngle[i] + te * (target - _fromAngle[i]));
    _writeServo(i, current);
  }

  if (t < 1.0f) return;  // frame still in progress

  // ---- Frame complete ----------------------------------------
  _snapshotFrom();

  switch (_phase) {

    case SS_PHASE_START:
      // Start frame done -> enter loop (or stop if halt was called early)
      if (_haltPending) {
        _haltPending = false;
        if (seq.stopFrame >= 0) {
          _phase = SS_PHASE_STOP;
          _startFrame((uint8_t)seq.stopFrame);
        } else {
          _phase = SS_PHASE_IDLE;
        }
      } else {
        _phase = SS_PHASE_LOOP;
        _startFrame(seq.loopFirst);
      }
      break;

    case SS_PHASE_LOOP:
      if (_frameIdx < seq.loopLast) {
        // Advance within loop body
        _startFrame(_frameIdx + 1);
      } else {
        // Reached end of loop body
        if (_haltPending) {
          _haltPending = false;
          if (seq.stopFrame >= 0) {
            _phase = SS_PHASE_STOP;
            _startFrame((uint8_t)seq.stopFrame);
          } else {
            _phase = SS_PHASE_IDLE;
          }
        } else {
          // Loop back to start of loop body
          _startFrame(seq.loopFirst);
        }
      }
      break;

    case SS_PHASE_STOP:
      // Stop frame done -> idle
      _phase = SS_PHASE_IDLE;
      break;

    default:
      break;
  }
}

// -- Debug -----------------------------------------------------

void ServoSequencer::printStatus(Stream& s) {
  const char* phaseNames[] = {"IDLE","START","LOOP","STOP"};
  const char* easingNames[] = {"LINEAR","EASE_IN","EASE_OUT","EASE_IN_OUT","SINE"};

  s.println(F("-- ServoSequencer -------------------"));
  s.print(F("  Sequences : ")); s.println(_seqCount);
  s.print(F("  Speed     : ")); s.println(_speed);
  s.print(F("  Easing    : ")); s.println(easingNames[_easing]);
  s.print(F("  Phase     : ")); s.println(phaseNames[_phase]);
  s.print(F("  Halting   : ")); s.println(_haltPending ? "yes" : "no");
  if (_phase != SS_PHASE_IDLE) {
    SS_Sequence& seq = _seq[_playIdx];
    s.print(F("  Sequence  : ")); s.println(seq.name);
    s.print(F("  Frame     : ")); s.print(_frameIdx);
    s.print(F("  (loop "));
    s.print(seq.loopFirst); s.print(F(".."));
    s.print(seq.loopLast);
    if (seq.startFrame >= 0) { s.print(F("  start=")); s.print(seq.startFrame); }
    if (seq.stopFrame  >= 0) { s.print(F("  stop="));  s.print(seq.stopFrame);  }
    s.println(F(")"));
  }
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++) {
    s.print(F("  Servo[")); s.print(i); s.print(F("]  pin="));
    s.print(_pin[i]); s.print(F("  angle="));
    s.print(_currentAngle[i]); s.print(F("deg  scale="));
    s.println(_scale[i]);
  }
  s.println(F("-------------------------------------"));
}

// -- Private ---------------------------------------------------

void ServoSequencer::_startFrame(uint8_t frameIdx) {
  _frameIdx   = frameIdx;
  _frameStart = millis();
  _snapshotFrom();
}

void ServoSequencer::_writeServo(uint8_t i, int16_t angle) {
  int16_t a = _clamp(i, angle);
  _currentAngle[i] = a;
  if (_servo[i].attached()) _servo[i].write((int)a);
}

int16_t ServoSequencer::_clamp(uint8_t i, int16_t a) const {
  if (a < _minAngle[i]) return _minAngle[i];
  if (a > _maxAngle[i]) return _maxAngle[i];
  return a;
}

int16_t ServoSequencer::_applyScale(uint8_t i, int16_t target) const {
  if (_scale[i] == 1.0f) return target;
  float scaled = _restAngle[i] + (target - _restAngle[i]) * _scale[i];
  return _clamp(i, (int16_t)scaled);
}

float ServoSequencer::_ease(float t) const {
  switch (_easing) {
    case SS_EASE_IN:     return t * t;
    case SS_EASE_OUT:    return t * (2.0f - t);
    case SS_EASE_IN_OUT: return (t < 0.5f) ? 2.0f*t*t : -1.0f + (4.0f - 2.0f*t)*t;
    case SS_SINE:        return 0.5f * (1.0f - cosf(t * 3.14159265f));
    default:             return t;
  }
}

int8_t ServoSequencer::_findByName(const char* name) const {
  for (uint8_t i = 0; i < _seqCount; i++)
    if (_seq[i].name && strcmp(_seq[i].name, name) == 0)
      return (int8_t)i;
  return -1;
}

void ServoSequencer::_snapshotFrom() {
  for (uint8_t i = 0; i < SS_NUM_SERVOS; i++)
    _fromAngle[i] = _currentAngle[i];
}
