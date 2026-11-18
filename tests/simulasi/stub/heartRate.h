// STUB: checkForBeat asli memakai pemrosesan sinyal; di sini cukup mengikuti skenario simulasi.
#ifndef STUB_HEARTRATE_H
#define STUB_HEARTRATE_H
#include "Arduino.h"
inline bool checkForBeat(int32_t) {
  bool b = sim().beat_now;
  sim().beat_now = false;
  return b;
}
#endif
