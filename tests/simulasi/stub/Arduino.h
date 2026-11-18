// STUB: pengganti Arduino.h untuk simulasi di komputer. Bukan library asli.
#ifndef STUB_ARDUINO_H
#define STUB_ARDUINO_H
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

typedef uint8_t byte;
#define PROGMEM
#define OUTPUT 1

struct LcdStub;  // diisi oleh stub LiquidCrystal_I2C

struct SimState {
  unsigned long millis = 0;
  uint32_t ir = 0;                    // nilai sensor IR yang "dibaca" sketch
  bool beat_now = false;              // diset simulasi, dibaca checkForBeat()
  std::vector<float> serial;          // nilai yang dicetak lewat Serial.println
  int tone_calls = 0;
  std::vector<unsigned long> tone_durasi;
  std::vector<unsigned long> delays;  // semua panggilan delay()
  std::vector<std::string> lcd_saat_delay;  // isi LCD setiap kali delay() dipanggil
  int oled_display_calls = 0;
  std::string oled_frame, oled_terakhir;
  LcdStub* lcd = nullptr;
};
inline SimState& sim() { static SimState s; return s; }

inline unsigned long millis() { return sim().millis; }
std::string lcd_teks();  // didefinisikan di LiquidCrystal_I2C.h
inline void delay(unsigned long ms) {
  sim().delays.push_back(ms);
  sim().lcd_saat_delay.push_back(lcd_teks());
  sim().millis += ms;
}
inline void pinMode(byte, byte) {}
inline void tone(byte, unsigned int, unsigned long durasi = 0) {
  sim().tone_calls++;
  sim().tone_durasi.push_back(durasi);
}
inline void noTone(byte) {}

struct SerialStub {
  void begin(long) {}
  void println(float v) { sim().serial.push_back(v); }
};
static SerialStub Serial;

#endif
