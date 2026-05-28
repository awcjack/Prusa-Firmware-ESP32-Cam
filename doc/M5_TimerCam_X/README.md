# M5Stack Timer Camera X — Prusa Connect Firmware

## Hardware

| Parameter | Value |
|---|---|
| MCU | ESP32-D0WDQ6-V3 (dual-core, 240 MHz) |
| Camera sensor | OV3660 (3 MP, up to 2048 × 1536) |
| PSRAM | 8 MB Quad |
| Flash | 4 MB |
| Battery | 140 mAh LiPo, monitored via GPIO 38 ADC |
| RTC | BM8563 on secondary I2C bus (SCL=GPIO 14, SDA=GPIO 12, addr=0x51) |
| Connectivity | Wi-Fi 802.11 b/g/n, Bluetooth 4.2 |
| USB | USB-C (debug / flash) |
| Expansion | HY2.0-4P connector |

## Differences from AI Thinker ESP32-CAM

| Feature | AI Thinker ESP32-CAM | M5 Timer Camera X |
|---|---|---|
| Camera sensor | OV2640 (2 MP) | OV3660 (3 MP) |
| XCLK frequency | 15 MHz | 20 MHz |
| PSRAM | 4 MB | 8 MB |
| SD card | Yes | No |
| Flash LED | Yes (GPIO 4) | No |
| Config reset button | Yes | No (serial console only) |
| Battery | No | 140 mAh LiPo |
| RTC | No | BM8563 |
| Power hold | N/A | GPIO 33 must stay HIGH |

## Camera GPIO Pinout

| Signal | GPIO |
|---|---|
| D0 (Y2) | 32 |
| D1 (Y3) | 35 |
| D2 (Y4) | 34 |
| D3 (Y5) | 5 |
| D4 (Y6) | 39 |
| D5 (Y7) | 18 |
| D6 (Y8) | 36 |
| D7 (Y9) | 19 |
| XCLK | 27 |
| PCLK | 21 |
| VSYNC | 22 |
| HREF | 26 |
| SCCB SDA | 25 |
| SCCB SCL | 23 |
| RESET | 15 |
| PWDN | — (not used) |

## RTC and Deep Sleep

The BM8563 RTC is initialised **before** WiFi so that:

- On deep-sleep wakeup the system clock is primed from the RTC before NTP
  completes, keeping EXIF timestamps accurate even when offline.
- After a successful NTP sync the verified time is written back to the RTC so
  subsequent wakeups start with a fresh reference.

After each photo is sent to Prusa Connect the firmware enters ESP32 deep sleep
for the configured refresh interval (default 5 min, maximum 1 h). The BM8563
alarm is **not** used for wakeup; the ESP32 internal timer wakeup is used
instead. The RTC is used solely for timekeeping.

## Battery

GPIO 38 is connected to the battery via a resistor divider. The firmware reads
it with a 64-sample median filter on every photo cycle and logs:

```
Battery: 3842 mV (72%)
```

The `/battery` HTTP endpoint returns a JSON object:

```json
{ "voltage_mv": 3842, "percent": 72, "low": false }
```

Battery voltage and percentage are also included in the main `/json_input`
response as `bat_mv`, `bat_percent`, and `bat_low`.

The device powers off if GPIO 33 (BAT_HOLD) goes LOW. The firmware asserts it
HIGH in the very first line of `setup()` before anything else runs.

## Arduino IDE Configuration

| Setting | Value |
|---|---|
| Board | ESP32 Arduino → **ESP32 Dev Module** |
| CPU Frequency | 240 MHz (WiFi/BT) |
| Flash Size | 4 MB (32 Mb) |
| Partition Scheme | Minimal SPIFFS (1.9 MB APP with OTA / 190 KB SPIFFS) |
| PSRAM | **Enabled** |

Enable `M5_TIMER_CAM_X` in `mcu_cfg.h` (all other board flags must be `false`).

## Flashing

Connect the USB-C cable. The device enumerates as a standard CP2104 serial port.
Hold the boot button while pressing reset, or use `esptool.py` with
`--before default_reset --after hard_reset`.

For first-time flashing enable **Erase All Flash Before Sketch Upload** under
Tools to clear any stale EEPROM data.

## OTA Firmware File

The Timer Camera X OTA binary is named `ESP32_PrusaConnectCam_M5TimerCamX.ino.bin`
and must be published as a separate GitHub release asset from the AI Thinker binary
(`ESP32_PrusaConnectCam.ino.bin`). Uploading the wrong binary will brick the device
because the GPIO assignments and camera sensor are incompatible.
