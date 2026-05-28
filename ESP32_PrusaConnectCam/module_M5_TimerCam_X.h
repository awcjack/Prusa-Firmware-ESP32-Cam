/**
   @file module_M5_TimerCam_X.h

   @brief Board definition for M5Stack Timer Camera-X (OV3660)

   Chip:    ESP32-D0WDQ6-V3
   Sensor:  OV3660 (3MP, 2048x1536, 66.5° DFOV)
   PSRAM:   8 MB
   Flash:   4 MB
   Battery: 140 mAh LiPo, monitored via GPIO 38 ADC
   RTC:     BM8563 on secondary I2C bus (SCL=14, SDA=12, addr=0x51)

   Arduino IDE board selection:
   Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module
   Tools -> CPU Frequency -> 240MHz (WiFi/BT)
   Tools -> Flash Size -> 4MB (32Mb)
   Tools -> Partition Scheme -> Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
   Tools -> PSRAM -> Enabled

   Differences vs AI Thinker ESP32-CAM:
   - OV3660 sensor (not OV2640) -> 20MHz XCLK, different JPEG validation
   - No SD card slot
   - No camera flash (GPIO 2 status LED only)
   - No config reset button
   - Battery powered: GPIO 33 must stay HIGH or device powers off
   - BM8563 RTC for accurate timestamps and timed deep sleep
*/

#pragma once

#include "mcu_cfg.h"

#ifdef M5_TIMER_CAM_X

/* --------------- CAMERA CFG -------------------*/
#define PWDN_GPIO_NUM               -1      ///< Not used on Timer Cam-X
#define RESET_GPIO_NUM              15      ///< Camera hardware reset
#define XCLK_GPIO_NUM               27      ///< External clock
#define SIOD_GPIO_NUM               25      ///< SCCB data
#define SIOC_GPIO_NUM               23      ///< SCCB clock
#define Y9_GPIO_NUM                 19      ///< Data D7
#define Y8_GPIO_NUM                 36      ///< Data D6
#define Y7_GPIO_NUM                 18      ///< Data D5
#define Y6_GPIO_NUM                 39      ///< Data D4
#define Y5_GPIO_NUM                  5      ///< Data D3
#define Y4_GPIO_NUM                 34      ///< Data D2
#define Y3_GPIO_NUM                 35      ///< Data D1
#define Y2_GPIO_NUM                 32      ///< Data D0
#define VSYNC_GPIO_NUM              22      ///< Vertical sync
#define HREF_GPIO_NUM               26      ///< Horizontal sync
#define PCLK_GPIO_NUM               21      ///< Pixel clock

/* OV3660 requires 20 MHz XCLK; overrides the 15 MHz default in camera.cpp */
#define CAMERA_XCLK_FREQ_HZ         20000000

/* ------------------ MCU CFG  ------------------*/
#define BOARD_NAME                  F("M5Stack Timer Camera-X")
#define ENABLE_BROWN_OUT_DETECTION  false   ///< Battery supply is stable; skip brown-out reset
#define ENABLE_PSRAM                true    ///< 8 MB PSRAM present

/* --------------- OTA UPDATE CFG  --------------*/
/* Board-specific binary name — MUST differ from AI Thinker "ESP32_PrusaConnectCam.ino.bin"
   to prevent accidental cross-flashing (different GPIOs, different sensor). */
#define OTA_UPDATE_FW_FILE          PSTR("ESP32_PrusaConnectCam_M5TimerCamX.ino.bin")
#define FW_STATUS_LED_PIN           2       ///< Status LED used during OTA flash
#define FW_STATUS_LED_LEVEL_ON      HIGH

/* --------------- FLASH LED CFG  ---------------*/
/* Timer Cam-X has no flash LED, only a PWM status LED on GPIO 2 */
#define ENABLE_CAMERA_FLASH         false
#define CAMERA_FLASH_DIGITAL_CTRL   false
#define CAMERA_FLASH_PWM_CTRL       false
#define CAMERA_FLASH_NEOPIXEL       false
#define FLASH_GPIO_NUM              -1
#define FLASH_NEOPIXEL_LED_PIN      -1
#define FLASH_OFF_STATUS            0
#define FLASH_ON_STATUS             0
#define FLASH_PWM_FREQ              2000
#define FLASH_PWM_CHANNEL           0
#define FLASH_PWM_RESOLUTION        8

/* --------------- SD CARD CFG  ---------------*/
/* Timer Cam-X has no SD card slot */
#define ENABLE_SD_CARD              false
#define SD_PIN_CLK                  -1
#define SD_PIN_CMD                  -1
#define SD_PIN_DATA0                -1

/* ---------- RESET CFG CONFIGURATION  ----------*/
/* No physical reset button; use serial console commands only */
#define CFG_RESET_PIN               -1
#define CFG_RESET_LED_PIN           2
#define CFG_RESET_LED_LEVEL_ON      HIGH

/* -------------- STATUS LED CFG ----------------*/
#define STATUS_LED_ENABLE           true
#define STATUS_LED_GPIO_NUM         2       ///< PWM-capable status LED
#define STATUS_LED_OFF_PIN_LEVEL    LOW

/* -------------- DHT SENSOR CFG ----------------*/
/* All GPIOs are occupied by camera / RTC / battery circuits */
#define DHT_SENSOR_ENABLE           false
#define DHT_SENSOR_PIN              -1

/* ----------- M5 TIMER CAM-X SPECIFIC ----------*/
#define BAT_HOLD_PIN                33      ///< HIGH = device stays on; LOW = power off
#define BAT_ADC_PIN                 38      ///< Battery voltage ADC (ADC1_CHANNEL_2)
#define BAT_ADC_SCALE               0.661f  ///< Resistor divider ratio
#define BAT_ADC_REF_MV              3600    ///< ADC reference baseline [mV]
#define BAT_FULL_MV                 4200    ///< Fully charged voltage [mV]
#define BAT_EMPTY_MV                3000    ///< Cut-off voltage [mV]
#define BAT_SAMPLE_COUNT            64      ///< Samples for median filter

#define RTC_SDA_PIN                 12      ///< BM8563 I2C SDA
#define RTC_SCL_PIN                 14      ///< BM8563 I2C SCL
#define RTC_I2C_ADDR                0x51    ///< BM8563 7-bit I2C address
#define RTC_I2C_FREQ                100000  ///< 100 kHz

/* Override EXIF sensor identification for OV3660 */
#undef CAMERA_MODEL
#define CAMERA_MODEL   "OV3660"

#endif  /* M5_TIMER_CAM_X */

/* EOF */
