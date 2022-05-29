#include "eink/wifi.h"

#include <Arduino.h>
#include <WiFi.h>

namespace eink {

int WiFiBegin(const char* SSID, const char* password) {
  unsigned long t0 = millis();
  WiFi.begin(SSID, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (millis() - t0 > EINK_WIFI_CONN_TIMEOUT_MS) {
      Serial.printf("Unable to connect to WiFi\n.");
      return -1;
    }
  }
  Serial.printf("Connected to WiFi: %s!\n", WiFi.localIP().toString().c_str());
  return 0;
}

int WiFiDisconnect() {
  WiFi.disconnect(true);
  return 0;
}

}  // namespace eink