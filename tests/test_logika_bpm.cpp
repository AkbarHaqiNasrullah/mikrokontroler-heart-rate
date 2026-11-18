// Uji logika murni.  Jalankan dari folder utama repo:
//   g++ -std=c++11 -Wall -Wextra -o /tmp/uji_logika tests/test_logika_bpm.cpp && /tmp/uji_logika
#include <cstdio>
#include <cstring>
#include "../detak_jantung/logika_bpm.h"

static int gagal = 0, total = 0;
#define CEK(kondisi) do { total++; if (!(kondisi)) { gagal++; std::printf("GAGAL baris %d: %s\n", __LINE__, #kondisi); } } while (0)
#define CEK_STR(a, b) do { total++; if (std::strcmp((a), (b)) != 0) { gagal++; std::printf("GAGAL baris %d: \"%s\" != \"%s\"\n", __LINE__, (a), (b)); } } while (0)

int main() {
  // Batas setiap kategori (tabel laki-laki 18-25)
  CEK_STR(bpm::status(48),  "Di Bawah Normal");
  CEK_STR(bpm::status(49),  "Sangat Baik++");
  CEK_STR(bpm::status(55),  "Sangat Baik++");
  CEK_STR(bpm::status(56),  "Sangat Baik");
  CEK_STR(bpm::status(61),  "Sangat Baik");
  CEK_STR(bpm::status(62),  "Baik");
  CEK_STR(bpm::status(85),  "Baik");
  CEK_STR(bpm::status(86),  "Cukup++");
  CEK_STR(bpm::status(89),  "Cukup++");
  CEK_STR(bpm::status(90),  "Cukup");
  CEK_STR(bpm::status(93),  "Cukup");
  CEK_STR(bpm::status(94),  "Kurang Baik");
  CEK_STR(bpm::status(101), "Kurang Baik");
  CEK_STR(bpm::status(102), "Buruk");
  CEK_STR(bpm::status(200), "Buruk");
  // Semua teks status muat di LCD 16 karakter
  for (int i = 0; i <= 260; i++) CEK(std::strlen(bpm::status(i)) <= 16);

  // Filter derau
  CEK(!bpm::valid(20.0f));
  CEK(bpm::valid(20.5f));
  CEK(bpm::valid(72.0f));
  CEK(bpm::valid(254.9f));
  CEK(!bpm::valid(255.0f));
  CEK(!bpm::valid(0.0f));

  // Rata-rata bergerak (jendela 4)
  bpm::RataBergerak r;
  CEK(r.rata2() == 0 && r.terisi == 0);
  r.tambah(60); CEK(r.rata2() == 60);
  r.tambah(80); CEK(r.rata2() == 70);
  r.tambah(70); r.tambah(70); CEK(r.rata2() == 70);
  r.tambah(100);                // 60 tergeser keluar -> (80+70+70+100)/4 = 80
  CEK(r.rata2() == 80 && r.terisi == 4);
  r.reset(); CEK(r.rata2() == 0);

  // Sesi 60 detak
  bpm::SesiUkur s;
  CEK(s.sisa() == 60 && s.rata2() == 0);
  for (int i = 0; i < 59; i++) CEK(!s.tambah(72.0f));
  CEK(s.sisa() == 1);
  CEK(s.tambah(72.0f));          // detak ke-60 -> penuh
  CEK(s.rata2() == 72);
  s.reset(); CEK(s.jumlah == 0 && s.sisa() == 60);
  // Pembulatan: 59 kali 70 dan 1 kali 71 -> 70.0166 -> 70 ; 30 kali 70 dan 30 kali 71 -> 70.5 -> 71
  for (int i = 0; i < 30; i++) s.tambah(70.0f);
  for (int i = 0; i < 30; i++) s.tambah(71.0f);
  CEK(s.rata2() == 71);

  std::printf("%d uji, %d gagal\n", total, gagal);
  return gagal ? 1 : 0;
}
