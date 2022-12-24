#include <Arduino.h>
#include <driver/adc.h>
#include <time.h>

#include <sstream>

#include "credentials.h"
#include "eink/ha_client.h"
#include "eink/time.h"
#include "eink/wifi.h"
#include "esp_sleep.h"
#include "esp_wifi.h"

#if defined(EINK_LILYGO)
#include "eink/lilygo/runner.h"
#define Runner eink::lilygo::LilygoRunner
#elif defined(EINK_TFT)
#include "eink/tft/runner.h"
#define Runner eink::tft::TFTRunner
#elif defined(EINK_M5PAPER)
#include "eink/m5paper/runner.h"
#define Runner eink::m5paper::M5PaperRunner
#else
#error No runner defined
#endif

RTC_DATA_ATTR int runs;

void start_deep_sleep(struct tm *now) {
  // 10 minutes.
  time_t sleep_for_s = 10 * 60;

  // If it's past midnight and before 6 am, sleep until 6am.
  if (now != nullptr && now->tm_hour < 6) {
    struct tm time_6am = *now;
    time_6am.tm_hour = 6;
    sleep_for_s = eink::ToEpoch(time_6am) - eink::ToEpoch(*now);
  }

  esp_sleep_enable_timer_wakeup(sleep_for_s * 1e6);
  esp_deep_sleep_start();
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

void setup() {
  time_t t0 = millis();

  Serial.begin(115200);

  adc_power_acquire();

  print_wakeup_reason();

  Serial.printf("Before wifi %ld\n", millis() - t0);
  if (eink::WiFiBegin(kWiFiSSID, kWiFiPass) != 0) {
    Serial.printf("Unable to connect to WiFi. Sleeping.\n");
    start_deep_sleep(nullptr);
  }
  Serial.printf("After wifi %ld\n", millis() - t0);
  eink::ConfigNTP();

  eink::HAData data;
  eink::HAClient hacli(kHomeAssistantAPIUrl, kHomeAssistantToken);
  hacli.FetchData(data);

  eink::HAClient hacli2(kHomeAssistant2APIUrl, kHomeAssistant2Token);
  hacli2.FetchData(data);

  Serial.printf("After getting data %ld\n", millis() - t0);

  Serial.printf("Before getting time %ld\n", millis() - t0);
  struct tm t = eink::GetCurrentTime();
  Serial.printf("After getting time %ld\n", millis() - t0);

  eink::WiFiDisconnect();
  esp_wifi_stop();
  adc_power_release();

  Runner runner;
  runner.Init();
  runner.Draw(data, t, runs);

  runs++;
  start_deep_sleep(&t);
}

void loop() {}