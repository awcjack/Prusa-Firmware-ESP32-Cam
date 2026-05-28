/**
   @file prusa_link.h

   @brief Prusa Link local API integration.

   Queries the printer's state via the legacy OctoPrint-compatible REST API
   using a static X-Api-Key header — no MD5 / Digest auth required.

   API key location:
     Prusa Connect web console → your printer → Settings → X-Api-Key

   Endpoint used:
     GET http://<printer_ip>/api/printer
     X-Api-Key: <api_key>

   This endpoint is supported by Prusa Link on MK4, XL, and MINI+ printers.
   If the printer IP is not configured the check is skipped and capture always runs.
*/

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "log.h"
#include "mcu_cfg.h"

/* Prefixed PS_ to avoid clashing with ESP32 HAL macros
   (esp32-hal-gpio.h defines DISABLED 0x00, errno.h defines ERROR, etc.) */
enum class PrinterState : uint8_t {
  PS_DISABLED,      ///< No printer IP configured — check skipped
  PS_OFFLINE,       ///< HTTP request failed (printer off or unreachable)
  PS_OPERATIONAL,   ///< Printer on, idle
  PS_PRINTING,      ///< Actively printing
  PS_PAUSED,        ///< Print paused
  PS_ERROR,         ///< Printer in error state
  PS_UNKNOWN,       ///< Connected but state unrecognised
};

class PrusaLink {
public:
  explicit PrusaLink(Logs* i_log);

  void    Init(String ip, String apiKey);
  void    SetIp(String ip)         { PrinterIp = ip; }
  void    SetApiKey(String apiKey) { ApiKey = apiKey; }
  String  GetIp()                  { return PrinterIp; }
  String  GetApiKey()              { return ApiKey; }

  PrinterState QueryPrinterState();
  String       StateToString(PrinterState state);
  bool         IsConfigured()      { return PrinterIp.length() > 0 && ApiKey.length() > 0; }

private:
  Logs*   log;
  String  PrinterIp;
  String  ApiKey;
};

extern PrusaLink SystemPrusaLink;

/* EOF */
