#pragma once
#include <avr/pgmspace.h>

// ============================================================
// sin_lut.h
// Look-Up Table (LUT) Sinus 256 entry, disimpan di Flash (PROGMEM)
// Digunakan bersama oleh semua animasi agar tidak duplikat memori
//
// Nilai: sin(angle) * 256, untuk angle 0..255 (satu lingkaran penuh)
// Cara pakai:
//   int16_t s = (int16_t)pgm_read_word(&SIN_LUT[angle]);
//   int16_t c = (int16_t)pgm_read_word(&SIN_LUT[(uint8_t)(angle + 64)]);
// ============================================================

extern const int16_t SIN_LUT[256] PROGMEM;
