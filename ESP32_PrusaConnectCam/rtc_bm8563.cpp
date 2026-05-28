/**
   @file rtc_bm8563.cpp

   @brief BM8563 RTC driver implementation for M5Stack Timer Camera-X.
*/

#include "mcu_cfg.h"

#ifdef M5_TIMER_CAM_X

#include "rtc_bm8563.h"

/* BM8563 register addresses */
static constexpr uint8_t REG_CTRL1       = 0x00;
static constexpr uint8_t REG_CTRL2       = 0x01;
static constexpr uint8_t REG_SECONDS     = 0x02;
static constexpr uint8_t REG_MINUTES     = 0x03;
static constexpr uint8_t REG_HOURS       = 0x04;
static constexpr uint8_t REG_DAYS        = 0x05;
static constexpr uint8_t REG_WEEKDAYS    = 0x06;
static constexpr uint8_t REG_MONTHS      = 0x07;
static constexpr uint8_t REG_YEARS       = 0x08;
static constexpr uint8_t REG_MIN_ALARM   = 0x09;
static constexpr uint8_t REG_HOUR_ALARM  = 0x0A;
static constexpr uint8_t REG_DAY_ALARM   = 0x0B;
static constexpr uint8_t REG_WDAY_ALARM  = 0x0C;

/* Control_2 bit masks */
static constexpr uint8_t CTRL2_AIE = 0x02;  /* alarm interrupt enable */
static constexpr uint8_t CTRL2_AF  = 0x08;  /* alarm flag */

/* Alarm disable bit (set MSB to disable an alarm register) */
static constexpr uint8_t ALARM_DISABLE = 0x80;

RtcBm8563 SystemRtc(&SystemLog);

RtcBm8563::RtcBm8563(Logs* i_log) : log(i_log), rtcWire(0) {}

void RtcBm8563::Init() {
  rtcWire.begin(RTC_SDA_PIN, RTC_SCL_PIN, RTC_I2C_FREQ);

  /* clear STOP bit and TEST1 bit in Control_1 */
  writeReg(REG_CTRL1, 0x00);

  /* clear any stale alarm flag */
  ClearAlarm();

  log->AddEvent(LogLevel_Info, F("BM8563 RTC init"));
  log->AddEvent(LogLevel_Info, F("RTC running: "), String(IsRunning() ? "yes" : "no"));
}

bool RtcBm8563::IsRunning() {
  /* VL bit (bit 7) of seconds register is set when clock lost power */
  return !(readReg(REG_SECONDS) & 0x80);
}

bool RtcBm8563::GetTime(RtcTime& t) {
  rtcWire.beginTransmission(RTC_I2C_ADDR);
  rtcWire.write(REG_SECONDS);
  if (rtcWire.endTransmission(false) != 0) {
    log->AddEvent(LogLevel_Error, F("RTC GetTime: I2C error"));
    return false;
  }

  rtcWire.requestFrom((uint8_t)RTC_I2C_ADDR, (uint8_t)7);
  if (rtcWire.available() < 7) {
    log->AddEvent(LogLevel_Error, F("RTC GetTime: short read"));
    return false;
  }

  t.seconds = bcdDecode(rtcWire.read() & 0x7F);
  t.minutes = bcdDecode(rtcWire.read() & 0x7F);
  t.hours   = bcdDecode(rtcWire.read() & 0x3F);
  t.day     = bcdDecode(rtcWire.read() & 0x3F);
  rtcWire.read();  /* weekday — not used */
  uint8_t monthRaw = rtcWire.read();
  t.month   = bcdDecode(monthRaw & 0x1F);
  uint8_t yearRaw = rtcWire.read();
  t.year    = bcdDecode(yearRaw) + ((monthRaw & 0x80) ? 1900 : 2000);

  return true;
}

bool RtcBm8563::SetTime(const RtcTime& t) {
  rtcWire.beginTransmission(RTC_I2C_ADDR);
  rtcWire.write(REG_SECONDS);
  rtcWire.write(bcdEncode(t.seconds));
  rtcWire.write(bcdEncode(t.minutes));
  rtcWire.write(bcdEncode(t.hours));
  rtcWire.write(bcdEncode(t.day));
  rtcWire.write(0x00);  /* weekday */
  uint8_t century = (t.year >= 2000) ? 0x00 : 0x80;
  rtcWire.write(bcdEncode(t.month) | century);
  rtcWire.write(bcdEncode((uint8_t)(t.year % 100)));
  bool ok = (rtcWire.endTransmission() == 0);
  if (!ok) {
    log->AddEvent(LogLevel_Error, F("RTC SetTime: I2C error"));
  }
  return ok;
}

/**
 * Set a one-shot alarm `seconds` from now and enable the alarm interrupt.
 * Resolution is minutes; values under 60 s round up to 1 minute.
 * The alarm fires when the current time matches the target minute/hour.
 */
bool RtcBm8563::SetAlarmInSeconds(uint32_t seconds) {
  RtcTime now;
  if (!GetTime(now)) {
    return false;
  }

  uint32_t totalMinutes = now.hours * 60u + now.minutes + (seconds + 59u) / 60u;
  uint8_t  alarmHour   = (totalMinutes / 60u) % 24u;
  uint8_t  alarmMinute = totalMinutes % 60u;

  /* disable day/weekday alarm fields; enable minute + hour only */
  writeReg(REG_MIN_ALARM,  bcdEncode(alarmMinute));
  writeReg(REG_HOUR_ALARM, bcdEncode(alarmHour));
  writeReg(REG_DAY_ALARM,  ALARM_DISABLE);
  writeReg(REG_WDAY_ALARM, ALARM_DISABLE);

  /* enable alarm interrupt, clear any old alarm flag */
  uint8_t ctrl2 = readReg(REG_CTRL2);
  ctrl2 &= ~CTRL2_AF;
  ctrl2 |=  CTRL2_AIE;
  writeReg(REG_CTRL2, ctrl2);

  log->AddEvent(LogLevel_Info, "RTC alarm set for " + String(alarmHour) + ":" + String(alarmMinute));
  return true;
}

bool RtcBm8563::ClearAlarm() {
  uint8_t ctrl2 = readReg(REG_CTRL2);
  ctrl2 &= ~(CTRL2_AF | CTRL2_AIE);
  return writeReg(REG_CTRL2, ctrl2);
}

bool RtcBm8563::writeReg(uint8_t reg, uint8_t val) {
  rtcWire.beginTransmission(RTC_I2C_ADDR);
  rtcWire.write(reg);
  rtcWire.write(val);
  return rtcWire.endTransmission() == 0;
}

uint8_t RtcBm8563::readReg(uint8_t reg) {
  rtcWire.beginTransmission(RTC_I2C_ADDR);
  rtcWire.write(reg);
  rtcWire.endTransmission(false);
  rtcWire.requestFrom((uint8_t)RTC_I2C_ADDR, (uint8_t)1);
  return rtcWire.available() ? rtcWire.read() : 0xFF;
}

#endif /* M5_TIMER_CAM_X */

/* EOF */
