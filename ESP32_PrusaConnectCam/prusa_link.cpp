/**
   @file prusa_link.cpp

   @brief Prusa Link local API integration.

   Uses the legacy OctoPrint-compatible /api/printer endpoint which accepts
   a plain X-Api-Key header — no Digest / MD5 auth needed.

   Example response from GET /api/printer:
   {
     "state": {
       "text": "Printing",
       "flags": {
         "operational": true,
         "printing": true,
         "paused": false,
         "error": false,
         "ready": false
       }
     },
     "temperature": { ... }
   }
*/

#include "prusa_link.h"

#define PRUSA_LINK_TIMEOUT_MS  5000   ///< HTTP connect+read timeout [ms]
#define PRUSA_LINK_API_PATH    "/api/printer"

PrusaLink SystemPrusaLink(&SystemLog);

PrusaLink::PrusaLink(Logs* i_log) : log(i_log) {}

void PrusaLink::Init(String ip, String apiKey) {
  PrinterIp = ip;
  ApiKey    = apiKey;
  if (IsConfigured()) {
    log->AddEvent(LogLevel_Info, "Prusa Link configured: http://" + PrinterIp + PRUSA_LINK_API_PATH);
  } else {
    log->AddEvent(LogLevel_Info, F("Prusa Link not configured — printer state check disabled"));
  }
}

PrinterState PrusaLink::QueryPrinterState() {
  if (!IsConfigured()) {
    return PrinterState::DISABLED;
  }

  if (WiFi.status() != WL_CONNECTED) {
    log->AddEvent(LogLevel_Warning, F("Prusa Link: WiFi not connected, skipping query"));
    return PrinterState::OFFLINE;
  }

  HTTPClient http;
  String url = "http://" + PrinterIp + PRUSA_LINK_API_PATH;
  http.begin(url);
  http.addHeader("X-Api-Key", ApiKey);
  http.setTimeout(PRUSA_LINK_TIMEOUT_MS);

  int code = http.GET();
  if (code != 200) {
    log->AddEvent(LogLevel_Warning, "Prusa Link: HTTP " + String(code) + " from " + url);
    http.end();
    return (code < 0) ? PrinterState::OFFLINE : PrinterState::UNKNOWN;
  }

  String body = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    log->AddEvent(LogLevel_Warning, "Prusa Link: JSON parse error: " + String(err.c_str()));
    return PrinterState::UNKNOWN;
  }

  bool operational = doc["state"]["flags"]["operational"] | false;
  bool printing    = doc["state"]["flags"]["printing"]    | false;
  bool paused      = doc["state"]["flags"]["paused"]      | false;
  bool error       = doc["state"]["flags"]["error"]       | false;

  PrinterState state;
  if (!operational) {
    state = PrinterState::OFFLINE;
  } else if (error) {
    state = PrinterState::ERROR;
  } else if (printing) {
    state = PrinterState::PRINTING;
  } else if (paused) {
    state = PrinterState::PAUSED;
  } else {
    state = PrinterState::OPERATIONAL;
  }

  log->AddEvent(LogLevel_Info, "Prusa Link: printer state = " + StateToString(state));
  return state;
}

String PrusaLink::StateToString(PrinterState state) {
  switch (state) {
    case PrinterState::DISABLED:     return "disabled";
    case PrinterState::OFFLINE:      return "offline";
    case PrinterState::OPERATIONAL:  return "operational";
    case PrinterState::PRINTING:     return "printing";
    case PrinterState::PAUSED:       return "paused";
    case PrinterState::ERROR:        return "error";
    default:                         return "unknown";
  }
}

/* EOF */
