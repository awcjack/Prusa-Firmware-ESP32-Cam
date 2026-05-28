/**
   @file rtc_bm8563.h

   @brief BM8563 real-time clock driver for M5Stack Timer Camera-X.

   The BM8563 is connected on I2C bus 0 (Wire):
     SCL = RTC_SCL_PIN (GPIO 14)
     SDA = RTC_SDA_PIN (GPIO 12)
     Address = 0x51
   Bus 1 is reserved for the camera SCCB driver (sccb-ng) in arduino-esp32 3.x.

   Features used:
   - Read / set current time (used to stamp EXIF when NTP unavailable)
   - Set a one-shot alarm so the host can enter deep sleep and wake on alarm
   - Clear alarm flag on wakeup

   Register map (BM8563 datasheet):
     0x00  Control_1
     0x01  Control_2   (AIE=bit1, AF=bit3)
     0x02  VL_Seconds
     0x03  Minutes
     0x04  Hours
     0x05  Days
     0x06  Weekdays
     0x07  Months_Century
     0x08  Years
     0x09  Minute_Alarm
     0x0A  Hour_Alarm
     0x0B  Day_Alarm
     0x0C  Weekday_Alarm
*/

#pragma once

#include "mcu_cfg.h"

#ifdef M5_TIMER_CAM_X

#include <Arduino.h>
#include <Wire.h>
#include "mcu_cfg.h"
#include "module_templates.h"
#include "log.h"

struct RtcTime {
  uint8_t  seconds;
  uint8_t  minutes;
  uint8_t  hours;
  uint8_t  day;
  uint8_t  month;
  uint16_t year;
};

class RtcBm8563 {
public:
  explicit RtcBm8563(Logs* i_log);

  void Init();
  bool GetTime(RtcTime& t);
  bool SetTime(const RtcTime& t);

  /* Set a minutes-resolution alarm and enable the alarm interrupt output.
     Call ClearAlarm() after wakeup so the INT line de-asserts. */
  bool SetAlarmInSeconds(uint32_t seconds);
  bool ClearAlarm();

  bool IsRunning();

private:
  Logs*  log;
  TwoWire rtcWire;

  uint8_t  bcdEncode(uint8_t val) { return ((val / 10) << 4) | (val % 10); }
  uint8_t  bcdDecode(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }

  bool     writeReg(uint8_t reg, uint8_t val);
  uint8_t  readReg(uint8_t reg);
};

extern RtcBm8563 SystemRtc;

#endif /* M5_TIMER_CAM_X */

/* EOF */
