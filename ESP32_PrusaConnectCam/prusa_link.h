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

enum class PrinterState : uint8_t {
  DISABLED,      ///< No printer IP configured — check skipped
  OFFLINE,       ///< HTTP request failed (printer off or unreachable)
  OPERATIONAL,   ///< Printer on, idle
  PRINTING,      ///< Actively printing
  PAUSED,        ///< Print paused
  ERROR,         ///< Printer in error state
  UNKNOWN,       ///< Connected but state unrecognised
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
