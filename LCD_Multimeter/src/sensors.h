#pragma once

#include <Arduino.h>
#include "../config.h"

// ==========================================
// KALIBRASI (SESUAIKAN DENGAN ALAT UKUR ASLI)
// ==========================================

// Kalibrasi Tegangan (ZMPT101B)
float voltage_calibration = 733.33;

// Sensitivitas ACS712 20A adalah 100mV/A atau 0.100V/A (Untuk referensi 5V)
float current_sensitivity = 0.100;
// Kalibrasi Arus jika diperlukan (faktor pengali tambahan / penyesuaian)
float current_calibration = 1.0; 
// Offset / kelebihan pembacaan arus saat tanpa beban
float current_offset = 0.06;

// Variabel Penampung Nilai Sensonr (State)
static float _sensors_v = 0.0;
static float _sensors_i = 0.0;
static float _sensors_p = 0.0;

static unsigned long _sensors_last_read = 0;

// ─────────────────────────────────────────────────────────────────
//  FUNGSI ALGORITMA MURNI DARI USER
// ─────────────────────────────────────────────────────────────────

float measureVoltage() {
  int max_val = 0;
  int min_val = 1023;
  
  unsigned long start_time = millis();
  // Sampling gelombang AC selama 100ms (5 siklus penuh untuk frekuensi PLN 50Hz)
  // Mencari nilai puncak (peak) tertinggi dan terendah
  while ((millis() - start_time) < 100) {
    int val = analogRead(PIN_VOLT); // Menggunakan PIN dari config.h sesuai project Multimeter
    if (val > max_val) max_val = val;
    if (val < min_val) min_val = val;
  }

  // Menentukan selisih Peak-to-Peak (Vpp) dalam satuan nilai analog ADC
  float val_pp = max_val - min_val;
  
  // Konversi ADC Vpp ke nilai tegangan Vpp sesungguhnya (referensi Arduino 5V)
  float volt_pp = (val_pp * 5.0) / 1024.0;

  // Konversi tegangan Peak-to-Peak ke Root Mean Square (Vrms)
  // Rumus untuk gelombang sinus murni: Vrms = Vpp / (2 * akar(2)) atau Vpp * 0.35355
  float volt_rms = volt_pp * 0.35355;

  // Mengalikan Vrms dengan faktor pengaturan gain trimpot ZMPT101B untuk hitung voltase nyata
  float final_voltage = volt_rms * voltage_calibration;

  // Memfilter noise (batas filter ditingkatkan ke 30V untuk memblokir noise 13.9V)
  // Saat kabel PLN dicabut, tegangan "mengambang" (floating) akan diabaikan
  if (final_voltage < 30.0) {
    final_voltage = 0.0;
  }

  return final_voltage;
}

float measureCurrent() {
  unsigned long start_time;
  unsigned long count = 0;
  unsigned long sum = 0;

  // --- Tahap 1: Mencari nilai titik tengah gelombang murni (DC Offset) ---
  // Modul ACS712 secara teori mengapung di nilai analog 512 (2.5V). Namun dalam praktiknya 
  // tegangan USB kadang 4.9V / 5.1V yang membuat titik tengahnya bergeser.
  start_time = millis();
  while ((millis() - start_time) < 40) { // 40ms = 2 siklus penuh gelombang AC 50Hz (cukup untuk cari rata-rata)
    sum += analogRead(PIN_CURRENT); // Menggunakan PIN dari config.h
    count++;
  }
  if (count == 0) return 0.0;
  float dc_offset = (float)sum / count;

  // --- Tahap 2: Menghitung True RMS (Akar Nilai Kuadrat Rata-Rata) ---
  // Algoritma ini jauh lebih stabil dan tahan noise untuk beban kecil dari pada metode Peak-to-Peak.
  float sum_sq = 0;
  count = 0;
  start_time = millis();
  while ((millis() - start_time) < 100) { // 100ms = 5 siklus AC 50Hz
    float val = (float)analogRead(PIN_CURRENT);
    float centered = val - dc_offset;        // Menghilangkan offset DC
    sum_sq += (centered * centered);         // Mengkuadratkan nilai
    count++;
  }
  
  if (count == 0) return 0.0;

  // Root Mean Square dari sinyal ADC
  float adc_rms = sqrt(sum_sq / count);

  // Konversi rentang RMS ADC menuju tegangan Voltage RMS sesungguhnya
  float volt_rms = (adc_rms * 5.0) / 1024.0;
  
  // Konversi Voltage RMS ke Ampere menggunakan sensitivitas chip (100mV/A = 0.100V/A)
  float final_current = (volt_rms / current_sensitivity) * current_calibration;

  // Pemotongan offset manual
  final_current -= current_offset;

  // Filter Noise: Dengan True RMS, perhitungan stabil dan noise sangat minim.
  // Tapi karena versi chip ACS712 Anda punya batas 20A, pembacaan rentang miliAmpere jadi kurang peka.
  // Kita hilangkan noise di bawah 0.05 A (sekitar Beban di bawah 11 Watt akan dianggap 0)
  if (final_current < 0.05) {
    final_current = 0.0;
  }

  return final_current;
}


// ─────────────────────────────────────────────────────────────────
//  sensorsSetup()
// ─────────────────────────────────────────────────────────────────
void sensorsSetup() {
    pinMode(PIN_VOLT, INPUT);
    pinMode(PIN_CURRENT, INPUT);
}

// ─────────────────────────────────────────────────────────────────
//  sensorsForceRead() - Panggil fungsi User murni
// ─────────────────────────────────────────────────────────────────
void sensorsForceRead() {
    _sensors_v = measureVoltage();
    _sensors_i = measureCurrent();
    _sensors_p = _sensors_v * _sensors_i;
}

// ─────────────────────────────────────────────────────────────────
//  sensorsLoop() - Dipanggil secara berkala
// ─────────────────────────────────────────────────────────────────
void sensorsLoop() {
    // Baca sensor setiap 500ms
    if (millis() - _sensors_last_read >= 500) {
        sensorsForceRead();
        _sensors_last_read = millis();
    }
}

// ─────────────────────────────────────────────────────────────────
//  Getters
// ─────────────────────────────────────────────────────────────────
float sensorsGetRawRMSVoltage() { return 0.0; }
float sensorsGetRawRMSCurrent() { return 0.0; }
float sensorsGetVoltage()       { return _sensors_v; }
float sensorsGetCurrent()       { return _sensors_i; }
float sensorsGetPower()         { return _sensors_p; }
