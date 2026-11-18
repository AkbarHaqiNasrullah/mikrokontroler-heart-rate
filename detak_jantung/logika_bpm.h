// Logika murni perhitungan detak jantung (tanpa library Arduino),
// sehingga bisa diuji di komputer dengan g++ (lihat folder tests/).
#ifndef LOGIKA_BPM_H
#define LOGIKA_BPM_H

namespace bpm {

const int   JUMLAH_DETAK_SESI = 60;     // jumlah detak valid untuk satu hasil rata-rata
const int   JENDELA_RATA      = 4;      // rata-rata bergerak untuk angka BPM di OLED
const float BPM_MIN           = 20.0f;  // di luar rentang ini dianggap derau
const float BPM_MAX           = 255.0f;

inline bool valid(float nilai) { return nilai > BPM_MIN && nilai < BPM_MAX; }

// Kriteria denyut istirahat LAKI-LAKI usia 18-25 (Tabel I pada proposal).
// Panjang teks <= 16 karakter agar muat di satu baris LCD 16x2.
inline const char* status(int rata2) {
  if (rata2 < 49)   return "Di Bawah Normal";
  if (rata2 <= 55)  return "Sangat Baik++";
  if (rata2 <= 61)  return "Sangat Baik";
  if (rata2 <= 85)  return "Baik";
  if (rata2 <= 89)  return "Cukup++";
  if (rata2 <= 93)  return "Cukup";
  if (rata2 <= 101) return "Kurang Baik";
  return "Buruk";
}

// Rata-rata bergerak dari beberapa BPM terakhir (untuk tampilan OLED).
struct RataBergerak {
  int data[JENDELA_RATA];
  int terisi;
  int posisi;
  RataBergerak() { reset(); }
  void reset() { terisi = 0; posisi = 0; }
  void tambah(float nilai) {
    data[posisi] = (int)(nilai + 0.5f);
    posisi = (posisi + 1) % JENDELA_RATA;
    if (terisi < JENDELA_RATA) terisi++;
  }
  int rata2() const {
    if (terisi == 0) return 0;
    long jumlah = 0;
    for (int i = 0; i < terisi; i++) jumlah += data[i];
    return (int)(jumlah / terisi);
  }
};

// Satu sesi pengukuran: mengumpulkan JUMLAH_DETAK_SESI detak valid.
struct SesiUkur {
  int   jumlah;
  float total;
  SesiUkur() { reset(); }
  void reset() { jumlah = 0; total = 0.0f; }
  // true jika sesi sudah penuh setelah detak ini ditambahkan
  bool tambah(float nilai) {
    jumlah++;
    total += nilai;
    return jumlah >= JUMLAH_DETAK_SESI;
  }
  int sisa() const { return JUMLAH_DETAK_SESI - jumlah; }
  int rata2() const { return jumlah == 0 ? 0 : (int)(total / jumlah + 0.5f); }
};

}  // namespace bpm

#endif
