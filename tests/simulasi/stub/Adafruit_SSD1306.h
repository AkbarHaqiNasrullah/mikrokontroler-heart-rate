// STUB: merekam isi "layar" OLED sebagai teks.
#ifndef STUB_SSD1306_H
#define STUB_SSD1306_H
#include "Arduino.h"
#include "Wire.h"
#define SSD1306_SWITCHCAPVCC 2
#define WHITE 1
struct Adafruit_SSD1306 {
  Adafruit_SSD1306(int, int, TwoWire*, int) {}
  bool begin(uint8_t, uint8_t) { return true; }
  void clearDisplay() { sim().oled_frame.clear(); }
  void display() { sim().oled_display_calls++; sim().oled_terakhir = sim().oled_frame; }
  void drawBitmap(int, int, const unsigned char*, int, int, int) {}
  void setTextSize(int) {}
  void setTextColor(int) {}
  void setCursor(int, int) {}
  void println(const char* t) { sim().oled_frame += std::string(t) + "|"; }
  void println(int v) { sim().oled_frame += std::to_string(v) + "|"; }
};
#endif
