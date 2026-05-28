/**
   @file battery.cpp

   @brief Battery monitoring for M5Stack Timer Camera-X.

   The Timer Camera-X has a resistor divider on GPIO 38 (ADC1_CHANNEL_2).
   Raw ADC value is converted to millivolts using the scale factor and
   baseline reference stored in the board header.

   Formula (from M5Stack TimerCam-arduino):
     raw_mv  = (adc_raw / 4095.0) * 3300   // 12-bit ADC, 3.3V ref
     bat_mv  = raw_mv / BAT_ADC_SCALE       // undo divider

   Percentage is linearly mapped between BAT_EMPTY_MV and BAT_FULL_MV.
*/

#include "mcu_cfg.h"

#ifdef M5_TIMER_CAM_X

#include "battery.h"

#include <algorithm>

Battery SystemBattery(&SystemLog);

Battery::Battery(Logs* i_log) : log(i_log), VoltageMv(0), Percent(0) {}

void Battery::Init() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  log->AddEvent(LogLevel_Info, F("Battery monitoring init (GPIO 38)"));
  Update();
}

void Battery::Update() {
  VoltageMv = ReadAdcMedianMv();

  if (VoltageMv >= BAT_FULL_MV) {
    Percent = 100;
  } else if (VoltageMv <= BAT_EMPTY_MV) {
    Percent = 0;
  } else {
    Percent = (uint8_t)(((float)(VoltageMv - BAT_EMPTY_MV) / (float)(BAT_FULL_MV - BAT_EMPTY_MV)) * 100.0f);
  }

  log->AddEvent(LogLevel_Verbose, "Battery: " + String(VoltageMv) + " mV, " + String(Percent) + "%");
}

/* Take BAT_SAMPLE_COUNT ADC readings, sort them, return the median in mV. */
uint32_t Battery::ReadAdcMedianMv() {
  uint32_t samples[BAT_SAMPLE_COUNT];

  for (int i = 0; i < BAT_SAMPLE_COUNT; i++) {
    samples[i] = analogRead(BAT_ADC_PIN);
  }

  std::sort(samples, samples + BAT_SAMPLE_COUNT);
  uint32_t median_raw = samples[BAT_SAMPLE_COUNT / 2];

  /* raw → mV at ADC input */
  float adc_mv = (median_raw / 4095.0f) * 3300.0f;

  /* undo resistor divider to get actual battery voltage */
  float bat_mv = adc_mv / BAT_ADC_SCALE;

  return (uint32_t)bat_mv;
}

#endif /* M5_TIMER_CAM_X */

/* EOF */
