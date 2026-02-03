// --------------------------------------------------
//
// ESP32 Function Generator - Bluetooth Control
// Device: ESP32-WROOM-32D
// 
// App sends data in format: "F:1000,A:3.3,W:0"
// F: Frequency in Hz (e.g., 1000)
// A: Amplitude in Volts (e.g., 3.3)
// W: Wave type (0=Sine, 1=Square, 2=Triangle, 3=Sawtooth)
//
// DAC output on GPIO25 (DAC1) or GPIO26 (DAC2)
//
// --------------------------------------------------

#include "BluetoothSerial.h"
#include <math.h>

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

// Init Bluetooth Class
BluetoothSerial ESP_BT;

// DAC output pin (ESP32 has 2 DAC channels: GPIO25 and GPIO26)
const int DAC_PIN = 25;  // DAC1

// Function generator parameters
float frequency = 1000.0;      // Hz
float amplitude = 3.3;         // Volts (max 3.3V for ESP32)
int waveType = 0;              // 0=Sine, 1=Square, 2=Triangle, 3=Sawtooth

// Wave generation variables
unsigned long lastMicros = 0;
float phase = 0.0;
const float TWO_PI = 6.283185307179586;
const int DAC_MAX = 255;       // 8-bit DAC

// Buffer for receiving Bluetooth data
String receivedData = "";

void setup() {
  Serial.begin(115200);
  ESP_BT.begin("ESP32_FuncGen");  // Bluetooth device name
  
  Serial.println("ESP32 Function Generator Started");
  Serial.println("Waiting for Bluetooth connection...");
  
  // No need to set pinMode for DAC pins
  
  // Print initial settings
  printSettings();
}

void loop() {
  // -------------------- Receive Bluetooth Data ----------------------
  if (ESP_BT.available()) {
    char inChar = (char)ESP_BT.read();
    
    if (inChar == '\n' || inChar == '\r') {
      // Process complete message
      if (receivedData.length() > 0) {
        parseCommand(receivedData);
        receivedData = "";
      }
    } else {
      receivedData += inChar;
    }
  }
  
  // -------------------- Generate Waveform ----------------------
  generateWaveform();
}

void parseCommand(String command) {
  Serial.println("Received: " + command);
  
  // Parse format: "F:1000,A:3.3,W:0"
  int freqIndex = command.indexOf("F:");
  int ampIndex = command.indexOf("A:");
  int waveIndex = command.indexOf("W:");
  
  if (freqIndex != -1) {
    int commaIndex = command.indexOf(',', freqIndex);
    String freqStr = command.substring(freqIndex + 2, commaIndex);
    float newFreq = freqStr.toFloat();
    if (newFreq > 0 && newFreq <= 10000) {  // Limit to 10kHz
      frequency = newFreq;
    }
  }
  
  if (ampIndex != -1) {
    int commaIndex = command.indexOf(',', ampIndex);
    String ampStr = command.substring(ampIndex + 2, commaIndex);
    float newAmp = ampStr.toFloat();
    if (newAmp >= 0 && newAmp <= 3.3) {  // ESP32 max is 3.3V
      amplitude = newAmp;
    }
  }
  
  if (waveIndex != -1) {
    String waveStr = command.substring(waveIndex + 2);
    int newWave = waveStr.toInt();
    if (newWave >= 0 && newWave <= 3) {
      waveType = newWave;
      phase = 0.0;  // Reset phase when changing wave type
    }
  }
  
  printSettings();
  
  // Send confirmation back to app
  ESP_BT.println("OK");
}

void generateWaveform() {
  unsigned long currentMicros = micros();
  unsigned long deltaTime = currentMicros - lastMicros;
  
  // Calculate phase increment based on frequency
  float phaseIncrement = TWO_PI * frequency * deltaTime / 1000000.0;
  phase += phaseIncrement;
  
  // Keep phase in range [0, 2*PI]
  if (phase >= TWO_PI) {
    phase -= TWO_PI;
  }
  
  lastMicros = currentMicros;
  
  // Generate waveform value
  float normalizedValue = 0.0;  // Range: -1 to 1
  
  switch (waveType) {
    case 0:  // Sine wave
      normalizedValue = sin(phase);
      break;
      
    case 1:  // Square wave
      normalizedValue = (phase < PI) ? 1.0 : -1.0;
      break;
      
    case 2:  // Triangle wave
      if (phase < PI) {
        normalizedValue = -1.0 + (2.0 * phase / PI);
      } else {
        normalizedValue = 3.0 - (2.0 * phase / PI);
      }
      break;
      
    case 3:  // Sawtooth wave
      normalizedValue = -1.0 + (phase / PI);
      break;
  }
  
  // Scale to amplitude and offset to positive range
  // DAC outputs 0-3.3V, so we need to offset
  float voltage = (normalizedValue * amplitude / 2.0) + (amplitude / 2.0);
  
  // Convert to DAC value (0-255)
  int dacValue = constrain((int)(voltage * DAC_MAX / 3.3), 0, DAC_MAX);
  
  // Output to DAC
  dacWrite(DAC_PIN, dacValue);
}

void printSettings() {
  Serial.println("=== Current Settings ===");
  Serial.print("Frequency: "); Serial.print(frequency); Serial.println(" Hz");
  Serial.print("Amplitude: "); Serial.print(amplitude); Serial.println(" V");
  Serial.print("Wave Type: ");
  switch (waveType) {
    case 0: Serial.println("Sine"); break;
    case 1: Serial.println("Square"); break;
    case 2: Serial.println("Triangle"); break;
    case 3: Serial.println("Sawtooth"); break;
  }
  Serial.println("========================");
}
