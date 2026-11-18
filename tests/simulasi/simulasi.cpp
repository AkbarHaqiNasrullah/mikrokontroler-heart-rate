// Simulasi sketch detak_jantung.ino di komputer memakai library PALSU (folder stub/).
// Tujuannya memeriksa alur program (hitung mundur, hasil, reset saat jari diangkat),
// BUKAN menggantikan uji pada alat sungguhan.
//
// Jalankan dari folder utama repo:
//   g++ -std=c++11 -Wall -Wextra -I tests/simulasi/stub -o /tmp/simulasi tests/simulasi/simulasi.cpp && /tmp/simulasi
#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "LiquidCrystal_I2C.h"
#include "../../detak_jantung/detak_jantung.ino"

static int gagal = 0, total = 0;
static void cek(bool ok, const std::string& nama) {
  total++;
  if (!ok) gagal++;
  std::printf("[%s] %s\n", ok ? "LULUS" : "GAGAL", nama.c_str());
}

static unsigned long next_beat = 0;
// Satu putaran loop(); waktu maju 1 ms; detak dibuat tiap `periode` ms.
// Detak yang jatuh di dalam delay() (sketch "buta") dibuang, seperti pada sensor sungguhan.
static void putaran(unsigned long periode) {
  SimState& s = sim();
  unsigned long sebelum = s.millis;
  s.millis += 1;
  if (periode) {
    while (next_beat <= sebelum) next_beat += periode;
    if (next_beat <= s.millis) { s.beat_now = true; next_beat += periode; }
  }
  loop();
}
static void jari(bool ada) { sim().ir = ada ? 50000 : 0; }
static void tanpa_jari(int putar) { jari(false); for (int i = 0; i < putar; i++) putaran(0); }
static void mulai_detak(unsigned long periode) { next_beat = sim().millis + periode; }
// Jalankan sampai ada hasil (delay 2000) baru
static std::string sampai_hasil(unsigned long periode) {
  SimState& s = sim();
  size_t awal = s.lcd_saat_delay.size();
  unsigned long batas = s.millis + 120000;
  while (s.lcd_saat_delay.size() == awal && s.millis < batas) putaran(periode);
  return s.lcd_saat_delay.size() > awal ? s.lcd_saat_delay.back() : "";
}

int main() {
  SimState& s = sim();
  setup();
  cek(s.delays.size() == 1 && s.delays[0] == 3000, "setup(): hanya satu delay 3 detik untuk layar pembuka");

  // 1. Tanpa jari saat baru menyala
  tanpa_jari(300);
  cek(s.oled_terakhir.find("Letakkan Jari") != std::string::npos, "tanpa jari: OLED menampilkan 'Letakkan Jari'");
  cek(s.tone_calls == 0, "tanpa jari: buzzer diam");

  // 2. Satu sesi penuh pada 72 BPM (periode 833 ms)
  int display_awal = s.oled_display_calls;
  unsigned long t_awal = s.millis;
  jari(true);
  mulai_detak(833);
  std::string hasil = sampai_hasil(833);
  unsigned long durasi = s.millis - t_awal;
  cek(hasil.compare(0, 2, "72") == 0 && hasil.find("Status:") != std::string::npos,
      "sesi 72 BPM: LCD baris 1 menampilkan rata-rata 72 dan 'Status:'  -> \"" + hasil + "\"");
  cek(hasil.substr(17, 4) == "Baik", "sesi 72 BPM: status 'Baik'");
  cek(s.serial.size() == 60, "sesi 72 BPM: tepat 60 nilai BPM valid dikirim ke Serial (didapat " + std::to_string(s.serial.size()) + ")");
  bool semua_normal = true;
  for (float v : s.serial) if (v < 71.5f || v > 72.5f) semua_normal = false;
  cek(semua_normal, "sesi 72 BPM: semua nilai BPM ~72 (detak pertama tidak dihitung)");
  int frame = s.oled_display_calls - display_awal;
  cek(frame <= (int)(durasi / 90) + 2, "OLED digambar ulang paling cepat tiap ~100 ms (" + std::to_string(frame) + " kali dalam " + std::to_string(durasi) + " ms)");
  bool kedip100 = !s.tone_durasi.empty();
  for (unsigned long d : s.tone_durasi) if (d != 100) kedip100 = false;
  cek(kedip100 && s.tone_calls >= 60, "buzzer: tiap detak berbunyi 100 ms tanpa delay (non-blocking)");
  bool delay_ok = true;
  for (size_t i = 1; i < s.delays.size(); i++) if (s.delays[i] != 2000) delay_ok = false;
  cek(delay_ok, "satu-satunya delay di loop() adalah tampilan hasil 2 detik");

  // 3. Sesi berikutnya langsung dimulai, tanpa data lama dan tanpa BPM semu (~25)
  s.serial.clear();
  std::string hasil2 = sampai_hasil(833);
  bool normal2 = s.serial.size() == 60;
  for (float v : s.serial) if (v < 71.5f || v > 72.5f) normal2 = false;
  cek(normal2 && hasil2.compare(0, 2, "72") == 0, "sesi kedua: 60 nilai ~72, tidak ada BPM semu setelah jeda hasil");

  // 4. Jari diangkat di tengah sesi -> hitung mundur mulai dari awal
  for (int i = 0; i < 20; i++) putaran(833);   // sebagian sesi
  tanpa_jari(300);
  cek(s.oled_terakhir.find("Letakkan Jari") != std::string::npos, "jari diangkat: OLED kembali ke 'Letakkan Jari'");
  jari(true);
  mulai_detak(833);
  s.serial.clear();
  std::vector<std::string> riwayat;
  while (s.serial.size() < 2) { putaran(833); std::string t = lcd_teks(); if (riwayat.empty() || riwayat.back() != t) riwayat.push_back(t); }
  cek(riwayat.back().compare(0, 16, "58s ...Until Avg") == 0,
      "jari dipasang lagi: hitung mundur mulai dari awal (2 BPM valid -> 58 s)  -> \"" + riwayat.back().substr(0, 16) + "\"");
  cek(s.serial[0] > 71.5f && s.serial[0] < 72.5f, "jari dipasang lagi: BPM pertama ~72 (bukan selisih waktu sejak jari diangkat)");

  // 5. Sesi lain dengan 100 BPM (periode 600 ms) -> 'Kurang Baik'
  tanpa_jari(300);
  jari(true);
  mulai_detak(600);
  std::string hasil3 = sampai_hasil(600);
  cek(hasil3.compare(0, 3, "100") == 0 && hasil3.substr(17, 11) == "Kurang Baik",
      "sesi 100 BPM: status 'Kurang Baik'  -> \"" + hasil3 + "\"");

  // 6. Sesi 55 BPM (periode 1091 ms -> 54.99 -> pembulatan 55) -> 'Sangat Baik++'
  tanpa_jari(300);
  jari(true);
  mulai_detak(1091);
  std::string hasil4 = sampai_hasil(1091);
  cek(hasil4.substr(17, 13) == "Sangat Baik++", "sesi ~55 BPM: status 'Sangat Baik++'  -> \"" + hasil4 + "\"");

  std::printf("\n%d pemeriksaan, %d gagal\n", total, gagal);
  return gagal ? 1 : 0;
}
