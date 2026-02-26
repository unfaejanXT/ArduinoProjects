/*
 * Project: VA_FREQ_DETECTOR
 * Description: Deteksi frekuensi listrik AC langsung dari sensor tegangan ZMPT101B.
 * 
 * Pinout:
 * ZMPT101B V3 -> A1
 */

#define VOLTSENSOR_PIN A1

void setup() {
  Serial.begin(9600);
  Serial.println("====================================");
  Serial.println("  VA FREQ DETECTOR - Inisialisasi   ");
  Serial.println("====================================");
  delay(1000);
}

void loop() {
  unsigned long sampleDuration = 1000; // Durasi deteksi (1000 ms = 1 detik)
  unsigned long startTime = millis();
  
  // Nilai 512 = 2.5V sebagai titik tengah gelombang sinus.
  int threshold = 512;  
  // Hysteresis (Toleransi batas) dinaikkan sedikit untuk menstabilkan noise/gelombang kotor
  int hysteresis = 30;  
  
  int cycleCount = 0;
  bool stateHigh = analogRead(VOLTSENSOR_PIN) > threshold;

  unsigned long firstCrossMicros = 0;
  unsigned long lastCrossMicros = 0;

  // Melakukan perulangan selama 1 detik penuh
  while (millis() - startTime < sampleDuration) {
    int val = analogRead(VOLTSENSOR_PIN);
    
    // Konsep Zero-crossing Software
    if (stateHigh) {
      if (val < (threshold - hysteresis)) {
        stateHigh = false;
        
        unsigned long currentMicros = micros();
        if (cycleCount == 0) {
          firstCrossMicros = currentMicros; // Menyimpan waktu gelombang menurun PERTAMA
        }
        lastCrossMicros = currentMicros;    // Menyimpan waktu gelombang menurun TERAKHIR
        cycleCount++; 
      }
    } else {
      if (val > (threshold + hysteresis)) {
        stateHigh = true;
      }
    }
  }
  
  // Hitung Frekuensi
  float frekuensi = 0.0;
  
  // Jika ada setidaknya 2 gelombang terekam (agar bisa diukur jarak waktu antar gelombangnya)
  if (cycleCount > 1) {
    // Total waktu (dalam mikrodetik) dari gelombang pertama ke gelombang terakhir
    unsigned long totalTimePeriod = lastCrossMicros - firstCrossMicros;
    
    // Rata-rata durasi/waktu 1 gelombang murni (dalam mikrodetik)
    float avgPeriodMicros = (float)totalTimePeriod / (cycleCount - 1);
    
    // Rumus F = 1 / T (Dikonversi dari mikrodetik ke detik)
    frekuensi = 1000000.0 / avgPeriodMicros;
  }
  
  // Filter jika listrik padam / tercabut
  if (frekuensi < 10.0 || frekuensi > 100.0) {
    frekuensi = 0.0;
  }
  
  Serial.print("Frekuensi Listrik : ");
  if (frekuensi > 0) {
    // Menampilkan dengan presisi 2 angka di belakang koma (misal: 50.15 Hz)
    Serial.print(frekuensi, 2);
  } else {
    Serial.print(frekuensi, 2);
  }
  Serial.println(" Hz");
}
