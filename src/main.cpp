#include <Arduino.h>
#include <driver/adc.h>
#include <time.h>

#include <sstream>

#include "credentials.h"
#include "eink/display.h"
#include "eink/ha_client.h"
#include "eink/logger.h"
#include "eink/time.h"
#include "eink/wifi.h"
#include "epd_driver.h"
#include "esp_adc_cal.h"
#include "esp_sleep.h"
#include "esp_wifi.h"

#define BATT_PIN 36

// Consider a sensor value stale if its older than this value.
#define STALE_THRESHOLD_S 2 * 3600
#define STALE_COLOR 180

#define SOIL_MOISTURE_ROWS 10
#define SOIL_MOISTURE_COLS 2

constexpr int kPadding = 10;

int vref = 1100;

RTC_DATA_ATTR int runs;

struct batt_t {
  double v;
  double percent;
};

batt_t get_battery_percentage() {
  // When reading the battery voltage, POWER_EN must be turned on
  epd_poweron();
  delay(50);

  uint16_t v = analogRead(BATT_PIN);
  epd_poweroff();

  double_t battery_voltage =
      ((double_t)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);

  double_t percent_experiment = ((battery_voltage - 3.7) / 0.5) * 100;

  // cap out battery at 100%
  // on charging it spikes higher
  if (percent_experiment > 100) {
    percent_experiment = 100;
  }

  batt_t res;
  res.v = battery_voltage;
  res.percent = percent_experiment;
  return res;
}

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

template <typename T>
std::string to_fixed_str(const T value, const int n = 0) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << value;
  return out.str();
}

void correct_adc_reference() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  }
}

int DrawHeader(eink::Display &display, const struct tm &now, int y) {
  int text_size = display.FontHeight(eink::FontSize::Size12);

  std::string tstring = eink::FormatTime(now);
  y += text_size;
  display.DrawText(y, kPadding, tstring.c_str(), 0, eink::FontSize::Size12,
                   eink::DrawTextDirection::LTR);

  batt_t batt = get_battery_percentage();
  std::string batt_str = to_fixed_str(batt.v, 2) + " V";

  display.DrawText(y, EPD_HEIGHT - kPadding, batt_str.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  return y;
}

void DrawFooter(eink::Display &display, time_t runtime_ms) {
  std::string txt = "Run #" + to_fixed_str(runs) + " took " +
                    to_fixed_str(runtime_ms / 1000.0, 1) + " s";
  display.DrawText(EPD_WIDTH - kPadding, EPD_HEIGHT - kPadding, txt.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
}

int DrawWeather(eink::Display &display, const eink::Weather &w, time_t now,
                int y) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int weather_size = display.FontHeight(eink::FontSize::Size24);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y + header_size + kPadding, kPadding, "Weather", 0,
                   eink::FontSize::Size16b);
  y += header_size + kPadding;

  int weather_y = y + kPadding + weather_size - kPadding;
  std::string val = to_fixed_str(w.temp, 1) + " °C";
  display.DrawText(weather_y, kPadding, val.c_str(), 0, eink::FontSize::Size24);

  int state_y = y + kPadding + text_size;
  display.DrawText(state_y, EINK_DISPLAY_HEIGHT - kPadding, w.state.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  display.DrawText(state_y + kPadding + text_size,
                   EINK_DISPLAY_HEIGHT - kPadding,
                   (eink::ToHumanDiff(now - w.last_updated) + " ago").c_str(),
                   0, eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  return weather_y;
}

int DrawTempCO2(eink::Display &display, const eink::Temp &t, const eink::CO2 &w,
                time_t now, int y) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int weather_size = display.FontHeight(eink::FontSize::Size24);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y + header_size + kPadding, kPadding, "Living Room", 0,
                   eink::FontSize::Size16b);
  y += header_size + kPadding;

  y += kPadding + weather_size - kPadding;
  std::string val = to_fixed_str(t.temp, 1) + " °C";
  display.DrawText(y, kPadding, val.c_str(), 0, eink::FontSize::Size24);

  std::string co2_val = to_fixed_str(w.ppm, 0) + " ppm";
  display.DrawText(y, EINK_DISPLAY_HEIGHT / 2, co2_val.c_str(), 0,
                   eink::FontSize::Size24);

  y += kPadding + text_size;
  display.DrawText(y, kPadding,
                   (eink::ToHumanDiff(now - t.last_updated) + " ago").c_str(),
                   0, eink::FontSize::Size12, eink::DrawTextDirection::LTR);
  display.DrawText(y, EINK_DISPLAY_HEIGHT / 2,
                   (eink::ToHumanDiff(now - w.last_updated) + " ago").c_str(),
                   0, eink::FontSize::Size12, eink::DrawTextDirection::LTR);
  return y;
}

int DrawSoilMoistures(eink::Display &display,
                      std::vector<eink::SoilMoisture> soil_moistures,
                      time_t now, int y0) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y0 + header_size + kPadding, kPadding, "b-parasites", 0,
                   eink::FontSize::Size16b);
  y0 += header_size + kPadding;

  int y_rows = y0;
  for (int i = 0; i < SOIL_MOISTURE_ROWS; i++) {
    display.DrawRect(y_rows + kPadding, 0, y_rows + 2 * kPadding + text_size,
                     EPD_HEIGHT, i % 2 ? 200 : 250);
    y_rows += kPadding + text_size;
  }

  // int y_soil = y;
  for (int i = 0; i < soil_moistures.size(); i++) {
    const eink::SoilMoisture &s = soil_moistures[i];
    // display.DrawRect(y + kPadding, 0, y + 2 * kPadding + text_size,
    // EPD_HEIGHT,
    //                  i % 2 ? 200 : 250);
    // y_soil += kPadding + text_size;

    int col = i / SOIL_MOISTURE_ROWS;
    int row = i % SOIL_MOISTURE_ROWS;

    int x = kPadding + col * EPD_HEIGHT / SOIL_MOISTURE_COLS;
    int y = y0 + kPadding + text_size + row * (kPadding + text_size);

    uint8_t text_color = now - s.last_updated > 2 * 3600 ? STALE_COLOR : 0;
    display.DrawText(y, x, s.name.c_str(), text_color, eink::FontSize::Size12);

    std::string val = s.error == "" ? (to_fixed_str(s.value, 0) + "%") : "?";
    // display.DrawText(y, EINK_DISPLAY_HEIGHT - kPadding, val.c_str(),
    // text_color,
    display.DrawText(y, x - 2 * kPadding + EPD_HEIGHT / SOIL_MOISTURE_COLS,
                     val.c_str(), text_color, eink::FontSize::Size12,
                     eink::DrawTextDirection::RTL);

    // std::string time = eink::ToHumanDiff(now - s.last_updated);
    // display.DrawText(y, EINK_DISPLAY_HEIGHT - kPadding - 150, time.c_str(),
    // 0,
    //                  eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  }

  return y_rows + kPadding;
}

void setup() {
  time_t t0 = millis();
  auto &logger = eink::Logger::Get();

  adc_power_acquire();

  correct_adc_reference();

  print_wakeup_reason();

  Serial.printf("Before wifi %ld\n", millis() - t0);
  if (eink::WiFiBegin(kWiFiSSID, kWiFiPass) != 0) {
    logger.Printf("Unable to connect to WiFi. Sleeping.\n");
    start_deep_sleep(nullptr);
  }
  Serial.printf("After wifi %ld\n", millis() - t0);
  eink::ConfigNTP();

  eink::HAClient hacli(kHomeAssistantAPIUrl, kHomeAssistantToken);
  eink::HAData data = hacli.FetchData();

  Serial.printf("After getting data %ld\n", millis() - t0);

  Serial.printf("Before getting time %ld\n", millis() - t0);
  struct tm t = eink::GetCurrentTime();
  Serial.printf("After getting time %ld\n", millis() - t0);

  eink::WiFiDisconnect();
  esp_wifi_stop();
  adc_power_release();

  eink::Display display;

  // display.DrawRect(0, 0, EINK_DISPLAY_WIDTH, EINK_DISPLAY_HEIGHT, 0);
  // display.Update();
  // delay(1000);
  // return;

  // // while (true) {
  // for (int i = 0; i < 2; i++) {
  //   display.DrawRect(0, 0, EINK_DISPLAY_WIDTH, EINK_DISPLAY_HEIGHT, 0);
  //   display.Update();
  //   delay(1000);

  //   display.Clear();
  //   display.Update();
  //   delay(1000);
  // }

  time_t now = eink::ToEpoch(t);

  int y = 0;

  Serial.printf("Before drawing %ld\n", millis() - t0);

  y = DrawHeader(display, t, y);
  y = DrawWeather(display, data.weather, now, y);
  y = DrawTempCO2(display, data.temp, data.co2, now, y);
  y = DrawSoilMoistures(display, data.soil_moistures, now, y);
  DrawFooter(display, millis() - t0);

  Serial.printf("Before updating %ld\n", millis() - t0);
  display.Update();
  Serial.printf("After updating %ld\n", millis() - t0);

  delay(2000);

  runs++;
  start_deep_sleep(&t);
}

void loop() {}