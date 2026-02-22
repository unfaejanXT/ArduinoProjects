# 🕐 LCD Jam Digital - NodeMCU v3

Proyek jam digital berbasis **NodeMCU v3 (ESP8266)** yang menampilkan **jam (HH:MM:SS)** dan **tanggal (DD-MM-YYYY)** di **LCD I2C 16×2**, disinkronkan secara otomatis via **NTP (Network Time Protocol)** menggunakan koneksi WiFi.

---

## 📁 Struktur Project

```
LCD_Jam_Digital/
├── LCD_Jam_Digital.ino        ← Entry point utama (SANGAT BERSIH)
├── config.h                    ← ⚙️ Konfigurasi (EDIT INI DULU!)
├── README.md                   ← Dokumentasi ini
└── src/
    ├── WiFiManager/
    │   ├── WiFiManager.h       ← Interface class WiFiManager
    │   └── WiFiManager.cpp     ← Implementasi koneksi WiFi
    ├── NTPManager/
    │   ├── NTPManager.h        ← Interface class NTPManager
    │   └── NTPManager.cpp      ← Implementasi sinkronisasi NTP
    ├── LCDDisplay/
    │   ├── LCDDisplay.h        ← Interface class LCDDisplay
    │   └── LCDDisplay.cpp      ← Implementasi tampilan LCD
    └── ClockApp/
        ├── ClockApp.h          ← Interface class ClockApp
        └── ClockApp.cpp        ← Orchestrator / State Machine
```

---

## ⚙️ Konfigurasi Awal (WAJIB)

Buka file **`config.h`** dan ubah bagian berikut:

```cpp
// 🔑 Ganti dengan data WiFi rumah Anda
#define WIFI_SSID       "NamaWiFiAnda"
#define WIFI_PASSWORD   "PasswordWiFiAnda"

// 🕐 Zona waktu (default: WIB = UTC+7)
#define NTP_UTC_OFFSET  25200   // WIB=25200, WITA=28800, WIT=32400

// 📺 Alamat I2C LCD (coba 0x3F jika 0x27 tidak berhasil)
#define LCD_I2C_ADDRESS  0x27
```

---

## 🛠️ Library yang Dibutuhkan

Install melalui **Arduino IDE → Tools → Manage Libraries**:

| Library             | Author             | Fungsi                     |
| ------------------- | ------------------ | -------------------------- |
| `NTPClient`         | Fabrice Weinberg   | Sinkronisasi waktu via NTP |
| `LiquidCrystal I2C` | Frank de Brabander | Driver LCD I2C             |

Library bawaan (sudah ada jika sudah install ESP8266):

- `ESP8266WiFi`
- `WiFiUdp`

---

## 🔧 Pengaturan Board Arduino IDE

| Setting       | Nilai                            |
| ------------- | -------------------------------- |
| Board         | **NodeMCU 1.0 (ESP-12E Module)** |
| Upload Speed  | 115200                           |
| CPU Frequency | 80 MHz                           |
| Flash Size    | 4MB (FS:2MB OTA:~1019KB)         |
| Port          | COM yang sesuai                  |

---

## 🔌 Skema Koneksi Hardware

### LCD I2C → NodeMCU v3

```
LCD I2C              NodeMCU v3
┌─────────┐         ┌──────────────┐
│  VCC    │ ──────► │  3.3V atau 5V│
│  GND    │ ──────► │  GND         │
│  SDA    │ ──────► │  D2 (GPIO4)  │
│  SCL    │ ──────► │  D1 (GPIO5)  │
└─────────┘         └──────────────┘
```

> **Catatan:** LCD I2C biasanya beroperasi di 5V. Jika menggunakan 3.3V dan LCD redup, coba gunakan pin VIN (5V dari USB) NodeMCU.

---

## 📺 Tampilan LCD

```
┌────────────────┐
│   12:34:56     │   ← Baris 1: Jam (HH:MM:SS) - Centered
│  22-02-2026    │   ← Baris 2: Tanggal (DD-MM-YYYY) - Centered
└────────────────┘
```

---

## 🔄 State Machine Aplikasi

```
  [BOOT]
     │
     ▼
 CONNECTING_WIFI ──(gagal)──► retry 5 detik
     │ (berhasil)
     ▼
 SYNCING_NTP ──(gagal 3x)──► CONNECTING_WIFI
     │ (berhasil)
     ▼
 RUNNING ◄──────────────────┐
     │ (WiFi putus)          │
     ▼                       │
 WIFI_LOST                   │
     │ (reconnect ok)         │
     └───► SYNCING_NTP ───────┘
```

---

## 🐛 Troubleshooting

### LCD tidak menampilkan apapun

1. Coba ganti `LCD_I2C_ADDRESS` dari `0x27` ke `0x3F` di `config.h`
2. Cek koneksi kabel SDA (D2) dan SCL (D1)
3. Putar potensiometer di belakang modul I2C untuk atur kontras

### Tidak bisa connect WiFi

1. Pastikan SSID dan password sudah benar di `config.h`
2. Pastikan router menggunakan frekuensi **2.4 GHz** (NodeMCU tidak support 5 GHz)
3. Buka Serial Monitor (115200 baud) untuk melihat log detail

### Waktu tidak akurat / salah zona waktu

1. Cek `NTP_UTC_OFFSET` di `config.h`
   - WIB = `25200` (UTC+7)
   - WITA = `28800` (UTC+8)
   - WIT = `32400` (UTC+9)

### Tanggal salah (tapi jam benar)

- Ini terjadi jika koneksi NTP pertama gagal. Tunggu siklus update berikutnya (1 jam) atau reset NodeMCU.

---

## 📊 Serial Monitor Output

```
================================================
  LCD Jam Digital - NodeMCU v3
  by: OOP Multi-File Project
================================================
[LCD] Inisialisasi berhasil.
[LCD] Ukuran: 16x2
[APP] State: CONNECTING_WIFI
Menghubungkan ke WiFi: NamaWiFiAnda
....
[WiFi] Terhubung!
[WiFi] IP Address : 192.168.1.100
[WiFi] RSSI       : -65 dBm
[APP] State: CONNECTING_WIFI → SYNCING_NTP
[NTP] Server  : pool.ntp.org
[NTP] Sinkronisasi berhasil! Waktu: 15:30:45
[APP] State: SYNCING_NTP → RUNNING
[CLOCK] 15:30:45  |  22-02-2026
```
