/*
   ESP8266 + FastLED + IR Remote: https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2016 Jason Coon

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

uint8_t power = 1;
uint8_t brightness = brightnessMap[brightnessIndex];

//String setPower(String value) {
//  power = value.toInt();
//  if(power < 0) power = 0;
//  else if (power > 1) power = 1;
//  return String(power);
//}

String getPower() {
  return String(power);
}

//String setBrightness(String value) {
//  brightness = value.toInt();
//  if(brightness < 0) brightness = 0;
//  else if (brightness > 255) brightness = 255;
//  return String(brightness);
//}

String getBrightness() {
  return String(brightness);
}

String getPattern() {
  return String(currentPatternIndex);
}

String getPatterns() {
  String json = "";

  for (uint8_t i = 0; i < patternCount; i++) {
    json += "\"" + patterns[i].name + "\"";
    if (i < patternCount - 1)
      json += ",";
  }

  return json;
}

String getPalette() {
  return String(currentPaletteIndex);
}

String getPalettes() {
  String json = "";

  for (uint8_t i = 0; i < paletteCount; i++) {
    json += "\"" + paletteNames[i] + "\"";
    if (i < paletteCount - 1)
      json += ",";
  }

  return json;
}

String getAutoplay() {
  return String(autoplay);
}

String getAutoplayDuration() {
  return String(autoplayDuration);
}

String getSolidColor() {
  return String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b);
}

String getCooling() {
  return String(cooling);
}

String getSparking() {
  return String(sparking);
}

String getSpeed() {
  return String(speed);
}

String getTwinkleSpeed() {
  return String(twinkleSpeed);
}

String getTwinkleDensity() {
  return String(twinkleDensity);
}

String getName() {
  return nameString;
}

String getRssi() {
  return String(state.rssi);
}

String getRssiRaw() {
  return String(state.rssiRaw);
}

String getRssiPeak() {
  return String(state.rssiPeak);
}

String getRssiPeakRaw() {
  return String(state.rssiPeakRaw);
}

String getRssiTrigger() {
  return String(state.rssiTrigger);
}

String getLap() {
  return String(lastPass.lap);
}

String getLapRssiPeak() {
  return String(lastPass.rssiPeak);
}

String getLapRSSIPeakRaw() {
  return String(lastPass.rssiPeakRaw);
}

String getLapTimeStamp() {
  return String(lastPass.timeStamp);
}

String getCalibrationMode() {
  return String(state.calibrationMode);
}

FieldList fields = {
  { "name", "Name", LabelFieldType, 0, 0, getName },
  { "power", "Power", BooleanFieldType, 0, 1, getPower },
  { "brightness", "Brightness", NumberFieldType, 1, 255, getBrightness },
  { "pattern", "Pattern", SelectFieldType, 0, patternCount, getPattern, getPatterns },
  { "palette", "Palette", SelectFieldType, 0, paletteCount, getPalette, getPalettes },
  { "speed", "Speed", NumberFieldType, 1, 255, getSpeed },
  
  { "race", "Race", SectionFieldType },
  { "frequency", "Frequency", SelectFieldType, 0, frequencyCount, getFrequency, getFrequencies},
  { "calibrationMode", "Calibrate", BooleanFieldType, 0, 1, getCalibrationMode },
  { "rssi", "RSSI", LabelFieldType, 0, 1024, getRssi },
  { "rssiRaw", "RSSI Raw", LabelFieldType, 0, 1024, getRssiRaw },
  { "rssiPeak", "RSSI Peak", LabelFieldType, 0, 1024, getRssiPeak },
  { "rssiPeakRaw", "RSSI Peak Raw", LabelFieldType, 0, 1024, getRssiPeakRaw },
  { "rssiTrigger", "RSSI Trigger", LabelFieldType, 0, 1024, getRssiTrigger },
  
  { "lap", "Lap", LabelFieldType, 0, 255, getLap },
  { "lapRssiPeak", "Lap RSSI Peak", LabelFieldType, 0, 255, getLapRssiPeak },
  { "lapRSSIPeakRaw", "Lap RSSI Peak Raw", LabelFieldType, 0, 255, getLapRSSIPeakRaw },
  { "lapTimeStamp", "Lap Time Stamp", LabelFieldType, 0, 255, getLapTimeStamp },
  
  { "autoplay", "Autoplay", SectionFieldType },
  { "autoplay", "Autoplay", BooleanFieldType, 0, 1, getAutoplay },
  { "autoplayDuration", "Autoplay Duration", NumberFieldType, 0, 255, getAutoplayDuration },
  { "solidColor", "Solid Color", SectionFieldType },
  { "solidColor", "Color", ColorFieldType, 0, 255, getSolidColor },
  { "fire", "Fire & Water", SectionFieldType },
  { "cooling", "Cooling", NumberFieldType, 0, 255, getCooling },
  { "sparking", "Sparking", NumberFieldType, 0, 255, getSparking },
  { "twinkles", "Twinkles", SectionFieldType },
  { "twinkleSpeed", "Twinkle Speed", NumberFieldType, 0, 8, getTwinkleSpeed },
  { "twinkleDensity", "Twinkle Density", NumberFieldType, 0, 8, getTwinkleDensity },
};

uint8_t fieldCount = ARRAY_SIZE(fields);
