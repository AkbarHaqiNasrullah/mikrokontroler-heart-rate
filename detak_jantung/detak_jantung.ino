/*
  Alat Pendeteksi Detak Jantung - Mata kuliah Mikrokontroler (Kelompok 2)
  Board : Arduino Uno
  Input : sensor MAX30102/MAX30105 (I2C)
  Output: OLED SSD1306 128x32 (BPM), LCD I2C 16x2 (hitung mundur & status), buzzer (pin 3)

  Library yang dibutuhkan (Library Manager): Adafruit GFX Library, Adafruit SSD1306,
  "SparkFun MAX3010x Pulse and Proximity Sensor Library", LiquidCrystal I2C.
  Alat ini proyek belajar, BUKAN alat medis.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <LiquidCrystal_I2C.h>
#include "logika_bpm.h"

// ---------- Pengaturan ----------
const byte          PIN_BUZZER     = 3;
const long          AMBANG_JARI    = 7000;  // nilai IR di atas ini = jari menempel
const unsigned long LAMA_KEDIP     = 100;   // ms, ikon detak & bunyi buzzer
const unsigned long JEDA_OLED      = 100;   // ms, OLED digambar ulang paling cepat tiap ini
const unsigned long LAMA_HASIL     = 2000;  // ms, hasil rata-rata ditampilkan di LCD

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
LiquidCrystal_I2C lcd(0x27, 16, 2);
MAX30105 particleSensor;

static const unsigned char PROGMEM Logo1[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E,  0x02, 0x10, 0x0C, 0x03, 0x10,
0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40,  0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
0x02, 0x08, 0xB8, 0x04,  0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
0x00,  0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,  };

static const unsigned char PROGMEM Logo2[] =
{ 0x01, 0xF0, 0x0F,  0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
0x20,  0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08,  0x03,
0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01,  0x80, 0x00, 0x14, 0x00,
0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40,  0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31,  0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
0x00, 0x60, 0xE0,  0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
0x01,  0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C,  0x00,
0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00,  0x00, 0x01, 0x80, 0x00  };

// ---------- Status ----------
bpm::SesiUkur     sesi;
bpm::RataBergerak rataOled;
unsigned long     detakTerakhir = 0;   // 0 = belum ada detak sebelumnya
unsigned long     kedipSampai   = 0;
unsigned long     oledTerakhir  = 0;
bool              jariAda       = true;   // true agar pesan "Letakkan Jari" tampil saat alat baru menyala

void gambarOled(bool kedip) {
  display.clearDisplay();
  if (kedip) display.drawBitmap(0, 0, Logo2, 32, 32, WHITE);
  else       display.drawBitmap(5, 5, Logo1, 24, 21, WHITE);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(50, 0);
  display.println("BPM");
  display.setCursor(50, 18);
  if (rataOled.terisi == 0) display.println("--");
  else                      display.println(rataOled.rata2());
  display.display();
}

void gambarTanpaJari() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 5);
  display.println("Letakkan Jari");
  display.setCursor(30, 15);
  display.println("Anda Di Sensor");
  display.setCursor(30, 25);
  display.println("Kelompok 2");
  display.display();
}

void tampilHitungMundur() {
  char baris[17];
  snprintf(baris, sizeof(baris), "%2ds ...Until Avg", sesi.sisa());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(baris);
}

void tampilHasil() {
  int rata2 = sesi.rata2();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(rata2);
  lcd.setCursor(5, 0);
  lcd.print("Status: ");
  lcd.setCursor(0, 1);
  lcd.print(bpm::status(rata2));
  delay(LAMA_HASIL);
}

void mulaiSesiBaru() {
  sesi.reset();
  rataOled.reset();
  detakTerakhir = 0;   // detak berikutnya hanya jadi patokan waktu, tidak dihitung
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BUZZER, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(3000);
  particleSensor.begin(Wire, I2C_SPEED_FAST);
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  lcd.init();
  lcd.backlight();
}

void loop() {
  long irValue = particleSensor.getIR();

  // --- Tidak ada jari: reset sesi agar data pengguna lama tidak tercampur ---
  if (irValue < AMBANG_JARI) {
    if (jariAda) {
      jariAda = false;
      mulaiSesiBaru();
      noTone(PIN_BUZZER);
      lcd.clear();
      gambarTanpaJari();
    }
    return;
  }

  if (!jariAda) {
    jariAda = true;
    mulaiSesiBaru();
  }

  // --- Deteksi detak: dipanggil di setiap putaran agar tidak ada detak terlewat ---
  if (checkForBeat(irValue)) {
    unsigned long sekarang = millis();
    tone(PIN_BUZZER, 1000, LAMA_KEDIP);     // non-blocking
    kedipSampai = sekarang + LAMA_KEDIP;

    if (detakTerakhir != 0) {
      float nilaiBpm = 60.0f / ((sekarang - detakTerakhir) / 1000.0f);
      if (bpm::valid(nilaiBpm)) {
        Serial.println(nilaiBpm);
        rataOled.tambah(nilaiBpm);
        bool penuh = sesi.tambah(nilaiBpm);
        if (penuh) {
          tampilHasil();
          mulaiSesiBaru();
          return;
        }
        tampilHitungMundur();
      }
    }
    detakTerakhir = sekarang;
  }

  // --- OLED digambar ulang berkala (bukan di setiap putaran) ---
  if (millis() - oledTerakhir >= JEDA_OLED) {
    oledTerakhir = millis();
    gambarOled(millis() < kedipSampai);
  }
}
