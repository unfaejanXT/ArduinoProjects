#pragma once

#include <Arduino.h>
#include "../config.h"

// ─────────────────────────────────────────────────────────────────
//  STATE INTERNAL SENSOR
// ─────────────────────────────────────────────────────────────────
static float _sensors_v_samples[SMOOTH_SAMPLES];
static float _sensors_i_samples[SMOOTH_SAMPLES];
static int   _sensors_sampleIdx   = 0;
static float _sensors_smoothV     = 0.0;
static float _sensors_smoothI     = 0.0;
static float _sensors_rawV        = 0.0;
static float _sensors_rawI        = 0.0;
static float _sensors_power       = 0.0;
static float _sensors_energy      = 0.0; // kWh atau Wh (opsional)

static float _sensors_rms_v_raw   = 0.0;
static float _sensors_rms_i_raw   = 0.0;

static unsigned long _sensors_lastRead = 0;
static unsigned long _sensors_lastTime = 0;

// ─────────────────────────────────────────────────────────────────
//  sensorsSetup()
// ─────────────────────────────────────────────────────────────────
void sensorsSetup() {
    pinMode(PIN_VOLT, INPUT);
    pinMode(PIN_CURRENT, INPUT);
    
    // Inisialisasi array untuk moving average (Verifikasi sensor)
    for (int i = 0; i < SMOOTH_SAMPLES; i++) {
        _sensors_v_samples[i] = 0.0;
        _sensors_i_samples[i] = 0.0;
    }
    _sensors_smoothV = 0.0;
    _sensors_smoothI = 0.0;
    _sensors_lastRead = millis();
    _sensors_lastTime = millis();
}

// ─────────────────────────────────────────────────────────────────
//  Hitung RMS
// ─────────────────────────────────────────────────────────────────
static void _sensorsReadRawRMS() {
    long sum_v_sq = 0;
    long sum_i_sq = 0;
    int samples = SAMPLES_PER_CYCLE;
    
    // Ambil sampel selama kurang lebih 1-2 siklus AC (20-40 ms)
    for (int i = 0; i < samples; i++) {
        long v_raw = analogRead(PIN_VOLT) - VOLT_OFFSET_RAW;
        long i_raw = analogRead(PIN_CURRENT) - CURRENT_OFFSET_RAW;
        
        sum_v_sq += v_raw * v_raw;
        sum_i_sq += i_raw * i_raw;
    }
    
    // Rata-rata kuadrat
    float avg_v_sq = (float)sum_v_sq / samples;
    float avg_i_sq = (float)sum_i_sq / samples;
    
    // Akar kuadrat untuk mendapatkan Nilai RMS ADC (belum dalam Volt/Ampere nyata)
    float rms_v_raw = sqrt(avg_v_sq);
    float rms_i_raw = sqrt(avg_i_sq);
    
    _sensors_rms_v_raw = rms_v_raw;
    _sensors_rms_i_raw = rms_i_raw;
    
    // Konversi ke Unit Nyata (Voltase)
    // Berbeda dari ACS, ZMPT tidak perlu dibagi 512, RMS raw-nya langsung dikali kalibrasi 
    // karena kalibrasi biasanya berdasarkan perbandingan Avometer / RMS ADC_Value (contoh: kalibrasi sekitar ~1.8 ke ~2.2)
    _sensors_rawV = rms_v_raw * VOLT_CALIBRATION; 
    
    // Konversi ke Unit Nyata (Arus)
    // Konversi ADC ke Voltage: (RMS raw / 1024.0) * 5.0
    float i_voltage = (rms_i_raw / 1024.0) * 5.0;
    _sensors_rawI   = i_voltage / CURRENT_CALIBRATION;
    
    // --------------------------------------------------------------------------
    // FILTER NOISE ADC (Mencegah Volt bocor saat mati / arus bocor 0.27A)
    // --------------------------------------------------------------------------
    // ZMPT1010B sering loncat 10 - 30V akibat float/noise kecil di pin analog.
    if (_sensors_rawV < 40.0) {
        _sensors_rawV = 0.0;
    }
    
    // Berdasarkan analisis LOG baru, batas RMS < 15.0 terlalu tinggi sehingga
    // kipas angin kecil ikut tertendang jadi 0. Kita rendahkan Clamp-nya kembali ke 0.19A
    // Jika arus yang dikalkulasikan (sebelum di-average) sangat kecil (di bawah 190 mA), jadikan 0
    if (_sensors_rawI < 0.19) {
        _sensors_rawI = 0.0;
    }
    
    // Jika tegangan alat sedang OFF (0V) / Relay mati, paksa Arus terindikasi sebagai 0
    if (_sensors_rawV == 0.0) {
        _sensors_rawI = 0.0;
    }
}

// ─────────────────────────────────────────────────────────────────
//  _sensorsMovingAverage
// ─────────────────────────────────────────────────────────────────
static void _sensorsMovingAverage() {
    _sensors_v_samples[_sensors_sampleIdx] = _sensors_rawV;
    _sensors_i_samples[_sensors_sampleIdx] = _sensors_rawI;
    
    _sensors_sampleIdx = (_sensors_sampleIdx + 1) % SMOOTH_SAMPLES;

    float sumV = 0.0;
    float sumI = 0.0;
    for (int i = 0; i < SMOOTH_SAMPLES; i++) {
        sumV += _sensors_v_samples[i];
        sumI += _sensors_i_samples[i];
    }
    
    _sensors_smoothV = sumV / SMOOTH_SAMPLES;
    _sensors_smoothI = sumI / SMOOTH_SAMPLES;
    
    // Hitung power
    _sensors_power = _sensors_smoothV * _sensors_smoothI;
}

// ─────────────────────────────────────────────────────────────────
//  sensorsLoop()
// ─────────────────────────────────────────────────────────────────
void sensorsLoop() {
    unsigned long now = millis();
    if (now - _sensors_lastRead >= SENSOR_READ_MS) {
        _sensors_lastRead = now;
        
        _sensorsReadRawRMS();
        _sensorsMovingAverage();
        
        // Integrasi Daya menjadi Energi (Wh)
        float dt_hours = (now - _sensors_lastTime) / 3600000.0; // Konversi ms ke jam
        _sensors_energy += _sensors_power * dt_hours;
        _sensors_lastTime = now;
    }
}

// ─────────────────────────────────────────────────────────────────
//  GETTER FUNCTIONS
// ─────────────────────────────────────────────────────────────────
// Memaksa bacaan tanpa menunggu timer (Berguna untuk testing Looping)
void sensorsForceRead() {
    _sensorsReadRawRMS();
    _sensorsMovingAverage();
    _sensors_lastRead = millis();
}

float sensorsGetVoltage() { return _sensors_smoothV; }
float sensorsGetCurrent() { return _sensors_smoothI; }
float sensorsGetPower()   { return _sensors_power;   }
float sensorsGetEnergy()  { return _sensors_energy;  }
float sensorsGetRawRMSVoltage() { return _sensors_rms_v_raw; }
float sensorsGetRawRMSCurrent() { return _sensors_rms_i_raw; }
