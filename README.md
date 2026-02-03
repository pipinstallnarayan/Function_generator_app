# Function_generator_app
Function_generator_app


# ESP32 Function Generator - Bluetooth Control

## Overview
Two versions of ESP32 function generator code that can be controlled via MIT App Inventor:

1. **ESP32_Function_Generator.ino** - String-based protocol (more flexible)
2. **ESP32_Function_Generator_Simple.ino** - Numeric protocol (similar to original LED code)

## Hardware Setup

### Components
- ESP32-WROOM-32D (or similar)
- Output: GPIO25 (DAC1) - connects to oscilloscope or amplifier

### DAC Output
- The ESP32 has two 8-bit DAC channels:
  - **GPIO25** (DAC1) - Used in this code
  - **GPIO26** (DAC2) - Alternative option
- Output range: 0V to 3.3V
- Resolution: 256 levels (8-bit)

## Version 1: String Protocol

### Command Format
Send commands as: `F:1000,A:3.3,W:0`

- **F:** Frequency in Hz (1 to 10000)
- **A:** Amplitude in Volts (0 to 3.3)
- **W:** Wave type (0=Sine, 1=Square, 2=Triangle, 3=Sawtooth)

### Examples
```
F:1000,A:3.3,W:0  -> 1000Hz, 3.3V amplitude, Sine wave
F:500,A:2.0,W:1   -> 500Hz, 2.0V amplitude, Square wave
F:2000,A:1.5,W:2  -> 2000Hz, 1.5V amplitude, Triangle wave
```

### MIT App Inventor Setup
1. Add **BluetoothClient** component
2. Add **ListPicker** to select "ESP32_FuncGen" device
3. Add three **TextBox** components for Frequency, Amplitude, and Wave Type
4. Add **Button** to send command

**Button Click Event:**
```blocks
when Button_Send.Click
  set BluetoothClient1.SendText to 
    join "F:" TextBox_Frequency.Text 
         ",A:" TextBox_Amplitude.Text 
         ",W:" TextBox_WaveType.Text
         "\n"
```

## Version 2: Simple Numeric Protocol

### Command Format
Send single integer values:

| Range | Function | Example | Result |
|-------|----------|---------|--------|
| 1-4999 | Set Frequency (Hz) | 1000 | 1000 Hz |
| 5000-5033 | Set Amplitude | 5033 | 3.3V (max) |
|  |  | 5020 | 2.0V |
|  |  | 5015 | 1.5V |
|  |  | 5000 | 0V (min) |
| 9000 | Sine Wave | 9000 | Sine |
| 9001 | Square Wave | 9001 | Square |
| 9002 | Triangle Wave | 9002 | Triangle |
| 9003 | Sawtooth Wave | 9003 | Sawtooth |

### Amplitude Calculation
To set amplitude: `5000 + (volts × 10)`
- 0V = 5000
- 1.5V = 5015
- 2.0V = 5020
- 3.3V = 5033

### MIT App Inventor Setup
1. Add **BluetoothClient** component
2. Add **ListPicker** to connect
3. Add **Slider** components or **TextBox** for input
4. Add **Buttons** for each parameter

**Example - Set Frequency Button:**
```blocks
when Button_SetFreq.Click
  set BluetoothClient1.SendText to join TextBox_Frequency.Text "\n"
```

**Example - Wave Type Buttons:**
```blocks
when Button_Sine.Click
  set BluetoothClient1.SendText to "9000\n"

when Button_Square.Click
  set BluetoothClient1.SendText to "9001\n"

when Button_Triangle.Click
  set BluetoothClient1.SendText to "9002\n"

when Button_Sawtooth.Click
  set BluetoothClient1.SendText to "9003\n"
```

**Example - Amplitude Slider:**
```blocks
when Slider_Amplitude.PositionChanged
  set command to 5000 + (Slider_Amplitude.ThumbPosition × 10)
  set BluetoothClient1.SendText to join command "\n"
```

## Waveform Types

| Type | Value | Description |
|------|-------|-------------|
| Sine | 0 / 9000 | Smooth sinusoidal wave |
| Square | 1 / 9001 | Digital square wave |
| Triangle | 2 / 9002 | Linear ramp up and down |
| Sawtooth | 3 / 9003 | Linear ramp up, instant drop |

## Specifications

- **Frequency Range:** 1 Hz to 10 kHz (practical limit ~5 kHz for clean waveforms)
- **Amplitude Range:** 0V to 3.3V
- **Output Resolution:** 8-bit (256 levels)
- **DAC Update Rate:** Continuous (microsecond precision)

## Upload Instructions

1. Install ESP32 board support in Arduino IDE
2. Select **ESP32 Dev Module** as board
3. Select correct COM port
4. Upload the sketch
5. Open Serial Monitor (115200 baud) to see status
6. Connect via Bluetooth from MIT App Inventor app

## Testing

1. **Connect oscilloscope** to GPIO25 and GND
2. **Power ESP32** via USB
3. **Open Serial Monitor** to verify operation
4. **Connect Bluetooth** from phone app
5. **Send commands** and observe waveform on oscilloscope

## Troubleshooting

### No Bluetooth Connection
- Ensure Bluetooth is enabled on phone
- Look for "ESP32_FuncGen" in device list
- Reset ESP32 and try again

### No Output on DAC
- Verify GPIO25 connection
- Check Serial Monitor for received commands
- Ensure amplitude is not set to 0V

### Distorted Waveforms
- Reduce frequency (try under 2 kHz)
- Check power supply stability
- Verify amplitude is within 0-3.3V range

## Notes

- ESP32 DAC is 8-bit, so expect some quantization
- Higher frequencies will have fewer samples per cycle
- For best results, keep frequency under 5 kHz
- Amplitude is peak-to-peak (centered around VCC/2)
- Output is DC-coupled (has DC offset)

