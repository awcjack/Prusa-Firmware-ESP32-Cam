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
#define PRUSA_LINK_API_PATH    "/api/v1/status"

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
    return PrinterState::PS_DISABLED;
  }

  if (WiFi.status() != WL_CONNECTED) {
    log->AddEvent(LogLevel_Warning, F("Prusa Link: WiFi not connected, skipping query"));
    return PrinterState::PS_OFFLINE;
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
    return (code < 0) ? PrinterState::PS_OFFLINE : PrinterState::PS_UNKNOWN;
  }

  String body = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    log->AddEvent(LogLevel_Warning, "Prusa Link: JSON parse error: " + String(err.c_str()));
    return PrinterState::PS_UNKNOWN;
  }

  /* Modern Prusa Link /api/v1/status response:
     { "printer": { "state": "PRINTING" | "IDLE" | "READY" | "PAUSED" |
                              "STOPPED" | "ATTENTION" | "BUSY" | "ERROR" | "FINISHED" },
       "job": { ... } }
  */
  if (!doc.containsKey("printer")) {
    log->AddEvent(LogLevel_Warning, F("Prusa Link: missing 'printer' key in response"));
    return PrinterState::PS_UNKNOWN;
  }

  String stateStr = doc["printer"]["state"] | "";
  stateStr.toUpperCase();

  PrinterState state;
  if (stateStr == "PRINTING" || stateStr == "BUSY") {
    state = PrinterState::PS_PRINTING;
  } else if (stateStr == "PAUSED") {
    state = PrinterState::PS_PAUSED;
  } else if (stateStr == "IDLE" || stateStr == "READY" || stateStr == "FINISHED" || stateStr == "STOPPED") {
    state = PrinterState::PS_OPERATIONAL;
  } else if (stateStr == "ERROR" || stateStr == "ATTENTION") {
    state = PrinterState::PS_ERROR;
  } else {
    log->AddEvent(LogLevel_Warning, "Prusa Link: unrecognised state: " + stateStr);
    state = PrinterState::PS_UNKNOWN;
  }

  log->AddEvent(LogLevel_Info, "Prusa Link: printer state = " + StateToString(state));
  return state;
}

String PrusaLink::StateToString(PrinterState state) {
  switch (state) {
    case PrinterState::PS_DISABLED:     return "disabled";
    case PrinterState::PS_OFFLINE:      return "offline";
    case PrinterState::PS_OPERATIONAL:  return "operational";
    case PrinterState::PS_PRINTING:     return "printing";
    case PrinterState::PS_PAUSED:       return "paused";
    case PrinterState::PS_ERROR:        return "error";
    default:                         return "unknown";
  }
}

/* EOF */
