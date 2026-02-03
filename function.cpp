// --------------------------------------------------
//
// ESP32 Function Generator - Simplified Bluetooth Control
// Device: ESP32-WROOM-32D
// 
// App sends simple numeric commands:
// Format: Single integer value
//
// Examples:
// 1000  -> Set frequency to 1000 Hz
// 5001  -> Set amplitude to 1.0V (5000 + value*10)
// 9000  -> Set wave to Sine (9000 + wave_type)
// 9001  -> Set wave to Square
// 9002  -> Set wave to Triangle
// 9003  -> Set wave to Sawtooth
//
// Command format:
// 1-4999: Frequency in Hz
// 5000-5033: Amplitude (subtract 5000, divide by 10 for voltage 0-3.3V)
// 9000-9003: Wave type (subtract 9000: 0=Sine, 1=Square, 2=Triangle, 3=Sawtooth)
//
// --------------------------------------------------

#include "BluetoothSerial.h"
#include <math.h>

BluetoothSerial ESP_BT;

// DAC output pin
const int DAC_PIN = 25;  // DAC1 on ESP32

// Function generator parameters
float frequency = 1000.0;      // Hz
float amplitude = 3.3;         // Volts
int waveType = 0;              // 0=Sine, 1=Square, 2=Triangle, 3=Sawtooth

// Wave generation variables
unsigned long lastMicros = 0;
float phase = 0.0;
const float TWO_PI = 6.283185307179586;
const int DAC_MAX = 255;

void setup() {
  Serial.begin(115200);
  ESP_BT.begin("ESP32_FuncGen");
  
  Serial.println("ESP32 Function Generator");
  Serial.println("Commands:");
  Serial.println("  1-4999: Set frequency (Hz)");
  Serial.println("  5000-5033: Set amplitude (5000=0V, 5033=3.3V)");
  Serial.println("  9000: Sine, 9001: Square, 9002: Triangle, 9003: Sawtooth");
  
  printSettings();
}

void loop() {
  // -------------------- Receive Bluetooth Command ----------------------
  if (ESP_BT.available()) {
    int incoming = ESP_BT.readStringUntil('\n').toInt();
    
    if (incoming >= 1 && incoming <= 4999) {
      // Frequency command
      frequency = (float)incoming;
      Serial.print("Frequency set to: "); Serial.print(frequency); Serial.println(" Hz");
      ESP_BT.print("Freq: "); ESP_BT.println(frequency);
      
    } else if (incoming >= 5000 && incoming <= 5033) {
      // Amplitude command (5000 to 5033 maps to 0V to 3.3V)
      amplitude = (incoming - 5000) / 10.0;
      Serial.print("Amplitude set to: "); Serial.print(amplitude); Serial.println(" V");
      ESP_BT.print("Amp: "); ESP_BT.println(amplitude);
      
    } else if (incoming >= 9000 && incoming <= 9003) {
      // Wave type command
      waveType = incoming - 9000;
      phase = 0.0;  // Reset phase
      Serial.print("Wave type set to: ");
      switch (waveType) {
        case 0: Serial.println("Sine"); ESP_BT.println("Wave: Sine"); break;
        case 1: Serial.println("Square"); ESP_BT.println("Wave: Square"); break;
        case 2: Serial.println("Triangle"); ESP_BT.println("Wave: Triangle"); break;
        case 3: Serial.println("Sawtooth"); ESP_BT.println("Wave: Sawtooth"); break;
      }
    }
  }
  
  // -------------------- Generate Waveform ----------------------
  generateWaveform();
}

void generateWaveform() {
  unsigned long currentMicros = micros();
  unsigned long deltaTime = currentMicros - lastMicros;
  
  float phaseIncrement = TWO_PI * frequency * deltaTime / 1000000.0;
  phase += phaseIncrement;
  
  if (phase >= TWO_PI) {
    phase -= TWO_PI;
  }
  
  lastMicros = currentMicros;
  
  float normalizedValue = 0.0;
  
  switch (waveType) {
    case 0:  // Sine
      normalizedValue = sin(phase);
      break;
      
    case 1:  // Square
      normalizedValue = (phase < PI) ? 1.0 : -1.0;
      break;
      
    case 2:  // Triangle
      if (phase < PI) {
        normalizedValue = -1.0 + (2.0 * phase / PI);
      } else {
        normalizedValue = 3.0 - (2.0 * phase / PI);
      }
      break;
      
    case 3:  // Sawtooth
      normalizedValue = -1.0 + (phase / PI);
      break;
  }
  
  float voltage = (normalizedValue * amplitude / 2.0) + (amplitude / 2.0);
  int dacValue = constrain((int)(voltage * DAC_MAX / 3.3), 0, DAC_MAX);
  
  dacWrite(DAC_PIN, dacValue);
}

void printSettings() {
  Serial.println("=== Settings ===");
  Serial.print("Freq: "); Serial.print(frequency); Serial.println(" Hz");
  Serial.print("Amp: "); Serial.print(amplitude); Serial.println(" V");
  Serial.print("Wave: ");
  switch (waveType) {
    case 0: Serial.println("Sine"); break;
    case 1: Serial.println("Square"); break;
    case 2: Serial.println("Triangle"); break;
    case 3: Serial.println("Sawtooth"); break;
  }
  Serial.println("================");
}
