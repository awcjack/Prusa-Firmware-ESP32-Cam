/**
   @file battery.h

   @brief Battery monitoring for M5Stack Timer Camera-X.
          Reads GPIO 38 ADC with a median filter and converts to mV / percentage.
          Compiled only when M5_TIMER_CAM_X is defined.
*/

#pragma once

#include "mcu_cfg.h"

#ifdef M5_TIMER_CAM_X

#include <Arduino.h>
#include "mcu_cfg.h"
#include "module_templates.h"
#include "log.h"

class Battery {
public:
  Battery(Logs* i_log);

  void    Init();
  void    Update();

  uint32_t GetVoltageMv()   const { return VoltageMv; }
  uint8_t  GetPercent()     const { return Percent; }
  bool     IsLow()          const { return VoltageMv < BAT_EMPTY_MV + 200; }

private:
  Logs*    log;
  uint32_t VoltageMv;
  uint8_t  Percent;

  uint32_t ReadAdcMedianMv();
};

extern Battery SystemBattery;

#endif /* M5_TIMER_CAM_X */

/* EOF */
