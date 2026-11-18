// STUB: LCD 16x2 sebagai dua baris teks.
#ifndef STUB_LCD_H
#define STUB_LCD_H
#include "Arduino.h"
struct LcdStub {
  char baris[2][17];
  int kol = 0, brs = 0;
  void bersihkan() { std::memset(baris, ' ', sizeof(baris)); baris[0][16] = baris[1][16] = 0; kol = brs = 0; }
};
struct LiquidCrystal_I2C {
  LcdStub s;
  LiquidCrystal_I2C(uint8_t, uint8_t, uint8_t) { s.bersihkan(); sim().lcd = &s; }
  void init() {}
  void backlight() {}
  void clear() { s.bersihkan(); }
  void setCursor(int c, int r) { s.kol = c; s.brs = r; }
  void print(const char* t) { for (; *t; ++t) if (s.kol < 16) s.baris[s.brs][s.kol++] = *t; }
  void print(int v) { print(std::to_string(v).c_str()); }
};
inline std::string lcd_teks() {
  if (!sim().lcd) return "";
  return std::string(sim().lcd->baris[0]) + "|" + sim().lcd->baris[1];
}
#endif
