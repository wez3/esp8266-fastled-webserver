// Boscam rx5808 FPV 5.8G wireless video receiver module

// Adapted for ESP8266 FastLED WebServer by Jason Coon
// Delta 5 Race Timer by Scott Chin
// SPI driver based on fs_skyrf_58g-main.c Written by Simon Chambers
// I2C functions by Mike Ochtman
//
// MIT License
//
// Copyright (c) 2017 Scott G Chin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Setup data pins for rx5808 comms
const int slaveSelectPin = D8; // 10
const int spiDataPin =     D7; // 11
const int spiClockPin =    D5; // 13

struct {
  uint16_t volatile vtxFreq = 5658;  // 5800

  // Subtracted from the peak rssi during a calibration pass to determine the trigger value
  uint16_t volatile calibrationOffset = 8;

  // Rssi must fall below trigger - settings.calibrationThreshold to end a calibration pass
  uint16_t volatile calibrationThreshold = 95;

  // Rssi must fall below trigger - settings.triggerThreshold to end a normal pass
  uint16_t volatile triggerThreshold = 40;

  uint8_t volatile filterRatio = 10;

  float volatile filterRatioFloat = 0.0f;
} settings;

struct {
  bool volatile calibrationMode = false;

  // True when the quad is going through the gate
  bool volatile crossing = false;

  // Current unsmoothed rssi
  uint16_t volatile rssiRaw = 0;

  // Smoothed rssi value, needs to be a float for smoothing to work
  float volatile rssiSmoothed = 0;

  // int representation of the smoothed rssi value
  uint16_t volatile rssi = 0;

  // rssi value that will trigger a new pass
  uint16_t volatile rssiTrigger;

  // The peak raw rssi seen the current pass
  uint16_t volatile rssiPeakRaw = 0;

  // The peak smoothed rssi seen the current pass
  uint16_t volatile rssiPeak = 0;

  // The time of the peak raw rssi for the current pass
  uint32_t volatile rssiPeakRawTimeStamp = 0;

  // variables to track the loop time
  uint32_t volatile loopTime = 0;
  uint32_t volatile lastLoopTimeStamp = 0;

  uint16_t volatile rssiMax = 0;
} state;

struct {
  uint16_t volatile rssiPeakRaw;
  uint16_t volatile rssiPeak;
  uint32_t volatile timeStamp;
  uint8_t volatile lap;
} lastPass;

typedef struct {
  String band;
  String name;
  int frequency;
} Frequency;
typedef Frequency FrequencyList[];

FrequencyList frequencies = {
  // IMD Channels
  { "IMD", "E2", 5685 },
  { "IMD", "F2", 5760 },
  { "IMD", "F4", 5800 },
  { "IMD", "F7", 5860 },
  { "IMD", "E6", 5905 },
  { "IMD", "E4", 5645 },

  // Band R - Raceband
  { "Raceband", "R1", 5658 },
  { "Raceband", "R2", 5695 },
  { "Raceband", "R3", 5732 },
  { "Raceband", "R4", 5769 },
  { "Raceband", "R5", 5806 },
  { "Raceband", "R6", 5843 },
  { "Raceband", "R7", 5880 },
  { "Raceband", "R8", 5917 },

  // Band F - ImmersionRC, Iftron
  { "ImmersionRC", "F1", 5740 },
  { "ImmersionRC", "F2", 5760 },
  { "ImmersionRC", "F3", 5780 },
  { "ImmersionRC", "F4", 5800 },
  { "ImmersionRC", "F5", 5820 },
  { "ImmersionRC", "F6", 5840 },
  { "ImmersionRC", "F7", 5860 },
  { "ImmersionRC", "F8", 5880 },

  // Band E - HobbyKing, Foxtech
  { "HobbyKing", "E1", 5705 },
  { "HobbyKing", "E2", 5685 },
  { "HobbyKing", "E3", 5665 },
  { "HobbyKing", "E4", 5645 },
  { "HobbyKing", "E5", 5885 },
  { "HobbyKing", "E6", 5905 },
  { "HobbyKing", "E7", 5925 },
  { "HobbyKing", "E8", 5945 },

  // Band B - FlyCamOne Europe
  { "FlyCamOne EU", "B1", 5733 },
  { "FlyCamOne EU", "B2", 5752 },
  { "FlyCamOne EU", "B3", 5771 },
  { "FlyCamOne EU", "B4", 5790 },
  { "FlyCamOne EU", "B5", 5809 },
  { "FlyCamOne EU", "B6", 5828 },
  { "FlyCamOne EU", "B7", 5847 },
  { "FlyCamOne EU", "B8", 5866 },

  // Band A - Team BlackSheep (TBS), RangeVideo, SpyHawk, FlyCamOne USA
  { "TBS", "A1", 5865 },
  { "TBS", "A2", 5845 },
  { "TBS", "A3", 5825 },
  { "TBS", "A4", 5805 },
  { "TBS", "A5", 5785 },
  { "TBS", "A6", 5765 },
  { "TBS", "A7", 5745 },
  { "TBS", "A8", 5725 },
};

int currentFrequencyIndex = 6;

const uint8_t frequencyCount = ARRAY_SIZE(patterns);

String getFrequency() {
  return String(currentFrequencyIndex);
}

String getFrequencies() {
  String json = "";

  for (uint8_t i = 0; i < frequencyCount; i++) {
    json += "\"" + frequencies[i].name + " - " + frequencies[i].band + "\"";
    if (i < frequencyCount - 1)
      json += ",";
  }

  return json;
}

// Functions for the rx5808 module
void SERIAL_SENDBIT1() {
  digitalWrite(spiClockPin, LOW);
  delayMicroseconds(300);
  digitalWrite(spiDataPin, HIGH);
  delayMicroseconds(300);
  digitalWrite(spiClockPin, HIGH);
  delayMicroseconds(300);
  digitalWrite(spiClockPin, LOW);
  delayMicroseconds(300);
}

void SERIAL_SENDBIT0() {
  digitalWrite(spiClockPin, LOW);
  delayMicroseconds(300);
  digitalWrite(spiDataPin, LOW);
  delayMicroseconds(300);
  digitalWrite(spiClockPin, HIGH);
  delayMicroseconds(300);
  digitalWrite(spiClockPin, LOW);
  delayMicroseconds(300);
}

void SERIAL_ENABLE_LOW() {
  delayMicroseconds(100);
  digitalWrite(slaveSelectPin, LOW);
  delayMicroseconds(100);
}

void SERIAL_ENABLE_HIGH() {
  delayMicroseconds(100);
  digitalWrite(slaveSelectPin, HIGH);
  delayMicroseconds(100);
}

// Calculate rx5808 register hex value for given frequency in MHz
uint16_t freqMhzToRegVal(uint16_t freqInMhz) {
  uint16_t tf, N, A;
  tf = (freqInMhz - 479) / 2;
  N = tf / 32;
  A = tf % 32;
  return (N << 7) + A;
}

// Set the frequency given on the rx5808 module
void setFrequency(int frequency) {
  uint8_t i; // Used in the for loops

  settings.vtxFreq = frequency;

  // Get the hex value to send to the rx module
  uint16_t vtxHex = freqMhzToRegVal(frequency);

  // bit bash out 25 bits of data / Order: A0-3, !R/W, D0-D19 / A0=0, A1=0, A2=0, A3=1, RW=0, D0-19=0
  SERIAL_ENABLE_HIGH();
  delay(2);
  SERIAL_ENABLE_LOW();
  SERIAL_SENDBIT0();
  SERIAL_SENDBIT0();
  SERIAL_SENDBIT0();
  SERIAL_SENDBIT1();
  SERIAL_SENDBIT0();

  for (i = 20; i > 0; i--) SERIAL_SENDBIT0(); // Remaining zeros

  SERIAL_ENABLE_HIGH(); // Clock the data in
  delay(2);
  SERIAL_ENABLE_LOW();

  // Second is the channel data from the lookup table, 20 bytes of register data are sent, but the
  // MSB 4 bits are zeros register address = 0x1, write, data0-15=vtxHex data15-19=0x0
  SERIAL_ENABLE_HIGH();
  SERIAL_ENABLE_LOW();

  SERIAL_SENDBIT1(); // Register 0x1
  SERIAL_SENDBIT0();
  SERIAL_SENDBIT0();
  SERIAL_SENDBIT0();

  SERIAL_SENDBIT1(); // Write to register

  // D0-D15, note: loop runs backwards as more efficent on AVR
  for (i = 16; i > 0; i--) {
    if (vtxHex & 0x1) { // Is bit high or low?
      SERIAL_SENDBIT1();
    }
    else {
      SERIAL_SENDBIT0();
    }
    vtxHex >>= 1; // Shift bits along to check the next one
  }

  for (i = 4; i > 0; i--) // Remaining D16-D19
    SERIAL_SENDBIT0();

  SERIAL_ENABLE_HIGH(); // Finished clocking data in
  delay(2);

  digitalWrite(slaveSelectPin, LOW);
  digitalWrite(spiClockPin, LOW);
  digitalWrite(spiDataPin, LOW);

  state.rssiMax = 0;
}


// Read the RSSI value for the current channel
int rssiRead() {
  return analogRead(0);
}

void setFrequencyIndex(int value) {
  currentFrequencyIndex = value;
  setFrequency(frequencies[currentFrequencyIndex].frequency);
}

void setCalibrationMode(int value) {
  state.calibrationMode = value != 0;
  state.rssiTrigger = state.rssi - settings.calibrationOffset;
  lastPass.rssiPeakRaw = 0;
  lastPass.rssiPeak = 0;
  state.rssiPeakRaw = 0;
  state.rssiPeakRawTimeStamp = 0;
}

void setupRx5808() {
  pinMode (slaveSelectPin, OUTPUT); // RX5808 comms
  pinMode (spiDataPin, OUTPUT);
  pinMode (spiClockPin, OUTPUT);
  digitalWrite(slaveSelectPin, HIGH);

  // Initialize lastPass defaults
  settings.filterRatioFloat = settings.filterRatio / 1000.0f;
  state.rssi = 0;
  state.rssiTrigger = 0;
  lastPass.rssiPeakRaw = 0;
  lastPass.rssiPeak = 0;
  lastPass.lap = 0;
  lastPass.timeStamp = 0;

  setFrequencyIndex(currentFrequencyIndex);
}

void rxLoop() {
  // Calculate the time it takes to run the main loop
  uint32_t lastLoopTimeStamp = state.lastLoopTimeStamp;
  state.lastLoopTimeStamp = micros();
  state.loopTime = state.lastLoopTimeStamp - lastLoopTimeStamp;

  state.rssiRaw = rssiRead();
  state.rssiSmoothed = (settings.filterRatioFloat * (float)state.rssiRaw) + ((1.0f - settings.filterRatioFloat) * state.rssiSmoothed);

  state.rssi = (int)state.rssiSmoothed;

  if (state.rssi > state.rssiMax) {
    state.rssiMax = state.rssi;
  }

  EVERY_N_MILLIS(500) {
    broadcastInt("rssi", state.rssi);
    broadcastInt("rssiRaw", state.rssiRaw);
  }

  Serial.print(state.rssiRaw);
  Serial.print(" ");
  Serial.print(state.rssiTrigger);
  Serial.print(" ");
  Serial.print(state.rssiMax);
  Serial.print(" ");
  Serial.println(state.rssi);

  if (state.rssiTrigger > 0) {
    if (!state.crossing && state.rssi > state.rssiTrigger) {
      state.crossing = true; // Quad is going through the gate
      broadcastInt("crossing", state.crossing);
//      Serial.println("Crossing = True");
    }

    // Find the peak rssi and the time it occured during a crossing event
    // Use the raw value to account for the delay in smoothing.
    if (state.rssiRaw > state.rssiPeakRaw) {
      state.rssiPeakRaw = state.rssiRaw;
      state.rssiPeakRawTimeStamp = millis();
      broadcastInt("rssiPeakRaw", state.rssiPeakRaw);
    }

    if (state.crossing) {
      int triggerThreshold = settings.triggerThreshold;

      // If in calibration mode, keep raising the trigger value
      if (state.calibrationMode) {
        state.rssiTrigger = _max(state.rssiTrigger, state.rssi - settings.calibrationOffset);

        broadcastInt("rssiTrigger", state.rssiTrigger);

        // when calibrating, use a larger threshold
        triggerThreshold = settings.calibrationThreshold;
      }

      state.rssiPeak = max(state.rssiPeak, state.rssi);
      broadcastInt("rssiPeak", state.rssiPeak);

//      Serial.print(" ");
//      Serial.print(state.rssiPeak);

      // Make sure the threshold does not put the trigger below 0 RSSI
      // See if we have left the gate
      if ((state.rssiTrigger > triggerThreshold) &&
          (state.rssi < (state.rssiTrigger - triggerThreshold))) {
//        Serial.println("Crossing = False");
        lastPass.rssiPeakRaw = state.rssiPeakRaw;
        lastPass.rssiPeak = state.rssiPeak;
        lastPass.timeStamp = state.rssiPeakRawTimeStamp;
        lastPass.lap = lastPass.lap + 1;

        broadcastInt("lap", lastPass.lap);
        broadcastInt("lapRssiPeak", lastPass.rssiPeak);
        broadcastInt("lapRSSIPeakRaw", lastPass.rssiPeakRaw);
        broadcastInt("lapTimeStamp", lastPass.timeStamp);

        state.crossing = false;
        state.calibrationMode = false;
        state.rssiPeakRaw = 0;
        state.rssiPeak = 0;

        broadcastInt("crossing", state.crossing);
        broadcastInt("calibrationMode", state.calibrationMode);
        broadcastInt("rssiPeak", state.rssiPeak);
        broadcastInt("rssiPeakRaw", state.rssiPeakRaw);
      }
    }
  }
}
