# LCD Multimeter Listrik Berbasis Arduino Uno

Project Multimeter Digital untuk mengukur **Tegangan (ZMPT1010B)** dan **Arus (ACS712)** listrik beraliran AC, dilengkapi dengan **Layar OLED**, **Buzzer**, **Button** (untuk menghidupkan dan mematikan alat secara keren), serta **Relay Module** (untuk memutuskan dan menyambungkan arus alat yang diukur).

## 🔥 Fitur Utama

- **Animasi Startup & Shutdown Keren**: Animasi kotak (OLED) membesar / mengecil untuk memberikan kesan _premium_ dan _clean_ ketika sistem dinyalakan atau dimatikan via tombol.
- **Transisi Melodi Buzzer**: Memberikan _feedback audio_ yang sinkron dengan animasi OLED.
- **Sistem Pembacaan Sensor Anti-Noise (Verifikasi Sensor)**:
  Project ini menggunakan dua lapis filtrasi yang telah diadaptasi dari project Anda sebelumnya (seperti di modul Ultrasonik) agar bacaan sensor _smooth_ dan tidak ada noise:
  1. **RMS Sampling (100 Sample per Siklus AC)**: Mengambil 100 sampel (mendekati 20-30 ms) dari gelombang AC ZMPT dan ACS712, lalu dirata-rata kuadratkan (RMS). Ini menolak lonjakan / _spikes_ instan pada pin Analog akibat riak frekuensi listrik AC.
  2. **Moving Average (Smoothing)**: Hasil RMS dimasukkan ke dalam _buffer_ array sebesar 5 sampel (didefinisikan di `SMOOTH_SAMPLES = 5`). Nilai yang ditampilkan pada OLED adalah rata-rata (Moving Average) dari 5 sampel RMS terakhir tersebut, memberikan nilai yang angkanya perubahannya stabil dan enak dilihat (tidak lompat-lompat drastis pada OLED).
- **Prosedural Paradigma**: Struktur C/C++ modern di Arduino (Pisah header `config.h` dan modul `src/` berisi internal state global statis) sehingga _main sketch_ `.ino` menjadi sangat bersih.

---

## 🔌 Spesifikasi & Wiring Pinout

- **Mikrokontroler**: Arduino Uno
- **Pasif Buzzer**: `D3`
- **Tombol / Btn0** (ON/OFF Toggle): `D2` _(Mode INPUT_PULLUP bawaan, hubungkan pin 1 ke D2, pin 2 ke GND)_
- **Relay Module**: `D4`
- **AC Voltage Sensor (ZMPT1010B LM358)**: `A2` (Analog Read)
- **AC Current Sensor (ACS712)**: `A3` (Analog Read)
- **OLED Display 128x64**: `A4` (SDA) dan `A5` (SCL) via I2C (`0x3C`)

---

## 📁 Struktur Direktori

```
LCD_Multimeter/
├── LCD_Multimeter.ino  <-- Main Arduino file, clean setup & loop.
├── README.md           <-- Dokumentasi ini.
├── config.h            <-- File satu pintu untuk mengatur konstanta/pin proyek.
└── src/
    ├── app.h           <-- Otak state-machine (Handle tombol, menyalakan, mematikan).
    ├── buzzer.h        <-- Modul pembunyian (Startup, shutdown, dan klik tombol).
    ├── display.h       <-- Modul OLED (Draw animasi keren dan Multimeter Interface).
    └── sensors.h       <-- Modul pembacaan Tegangan dan Arus + Perhitungan RMS.
```

---

## 💡 Analisa Verifikasi Sensor (Berdasarkan Konteks Sebelumnya)

Dari proyek-proyek Anda sebelumnya (misal, `LCD_Radar_Proximity_Ultrasonic`), _verifikasi pembacaan sensor_ yang Anda gunakan adalah dengan melakukan filter **Moving Average** sehingga tidak asal mengambil data tunggal. Ini diimplementasikan di dalam `sensors.h` LCD_Multimeter kali ini:

1. `analogRead()` dilakukan _ratusan kali_ selama 20-40 milidetik (mewakili 1 gelombang AC 50Hz) untuk mencari **RMS value**.
2. Nilai RMS ini **tidak** langsung ditendang dan ditampilkan, tetapi masuk dulu ke antrian (queue) sebesar 5 urutan terbawah (`SMOOTH_SAMPLES`). Array digeser sedemikian rupa seperti _Running Average_.
3. Hasil akhir adalah `_sensors_smoothV` dan `_sensors_smoothI` yang terkonfirmasi valid tanpa jitter / ghosting.
4. Konstanta offset `512` digunakan sebagai _mid-point_ di Arduino berbasis 5V (2.5V = 512).

_(Jika hasil di OLED berbeda dengan Avometer asli, silahkan tweak parameter kalibrasi pengali pada `config.h` di bagian `VOLT_CALIBRATION` dan `CURRENT_CALIBRATION`)_.

---

## 🚀 Instalasi Library

Pastikan Anda sudah menginstall library berikut di **Arduino IDE** (Ctrl + Shift + I):

- **Adafruit GFX Library**
- **Adafruit SSD1306**
- **Wire** (Sudah ada dari bawaan IDE)

Selamat berkreasi! 🛠️
