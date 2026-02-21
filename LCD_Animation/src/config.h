#pragma once

// ============================================================
// config.h
// Konfigurasi global untuk project LCD Animation
// ============================================================

// --- Display ---
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C

// --- Animasi ---
// Durasi setiap animasi sebelum ganti ke animasi berikutnya (ms)
#define ANIM_DURATION_MS  5000UL

// Jumlah total animasi yang terdaftar di AnimationManager
#define TOTAL_ANIMATIONS  4
