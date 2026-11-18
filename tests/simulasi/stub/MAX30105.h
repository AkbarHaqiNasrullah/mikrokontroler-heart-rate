#ifndef STUB_MAX30105_H
#define STUB_MAX30105_H
#include "Arduino.h"
#include "Wire.h"
struct MAX30105 {
  bool begin(TwoWire&, uint32_t) { return true; }
  void setup() {}
  void setPulseAmplitudeRed(byte) {}
  uint32_t getIR() { return sim().ir; }
};
#endif
