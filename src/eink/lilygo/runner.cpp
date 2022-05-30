#include "eink/lilygo/runner.h"

#include "eink/display_utils.h"
#include "eink/time.h"
#include "esp_adc_cal.h"

#define BATT_PIN 36

// Consider a sensor value stale if its older than this value.
#define STALE_THRESHOLD_S 2 * 3600
#define STALE_COLOR 180

#define SOIL_MOISTURE_ROWS 10
#define SOIL_MOISTURE_COLS 2

namespace eink {
namespace lilygo {
namespace {

int vref = 1100;

struct batt_t {
  double v;
  double percent;
};

void correct_adc_reference() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  }
}

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

constexpr int kPadding = 10;

int DrawHeader(eink::lilygo::Display &display, const struct tm &now, int y) {
  int text_size = display.FontHeight(eink::FontSize::Size12);

  std::string tstring = eink::FormatTime(now);
  y += text_size;
  display.DrawText(y, kPadding, tstring.c_str(), 0, eink::FontSize::Size12,
                   eink::DrawTextDirection::LTR);

  batt_t batt = get_battery_percentage();
  std::string batt_str = to_fixed_str(batt.v, 2) + " V";

  display.DrawText(y, EINK_DISPLAY_HEIGHT - kPadding, batt_str.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  return y;
}

void DrawFooter(eink::lilygo::Display &display, int n_runs) {
  std::string txt = "Run #" + to_fixed_str(n_runs);
  display.DrawText(EINK_DISPLAY_WIDTH - kPadding,
                   EINK_DISPLAY_HEIGHT - kPadding, txt.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
}

int DrawWeather(eink::lilygo::Display &display, const eink::Weather &w,
                time_t now, int y) {
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

int DrawTempCO2(eink::lilygo::Display &display, const eink::Temp &t,
                const eink::CO2 &w, time_t now, int y) {
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

int DrawSoilMoistures(eink::lilygo::Display &display,
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
                     EINK_DISPLAY_HEIGHT, i % 2 ? 200 : 250);
    y_rows += kPadding + text_size;
  }

  // int y_soil = y;
  for (int i = 0; i < soil_moistures.size(); i++) {
    const eink::SoilMoisture &s = soil_moistures[i];
    // display.DrawRect(y + kPadding, 0, y + 2 * kPadding + text_size,
    // EINK_DISPLAY_HEIGHT,
    //                  i % 2 ? 200 : 250);
    // y_soil += kPadding + text_size;

    int col = i / SOIL_MOISTURE_ROWS;
    int row = i % SOIL_MOISTURE_ROWS;

    int x = kPadding + col * EINK_DISPLAY_HEIGHT / SOIL_MOISTURE_COLS;
    int y = y0 + kPadding + text_size + row * (kPadding + text_size);

    uint8_t text_color = now - s.last_updated > 2 * 3600 ? STALE_COLOR : 0;
    display.DrawText(y, x, s.name.c_str(), text_color, eink::FontSize::Size12);

    std::string val = s.error == "" ? (to_fixed_str(s.value, 0) + "%") : "?";
    // display.DrawText(y, EINK_DISPLAY_HEIGHT - kPadding, val.c_str(),
    // text_color,
    display.DrawText(
        y, x - 2 * kPadding + EINK_DISPLAY_HEIGHT / SOIL_MOISTURE_COLS,
        val.c_str(), text_color, eink::FontSize::Size12,
        eink::DrawTextDirection::RTL);

    // std::string time = eink::ToHumanDiff(now - s.last_updated);
    // display.DrawText(y, EINK_DISPLAY_HEIGHT - kPadding - 150, time.c_str(),
    // 0,
    //                  eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  }

  return y_rows + kPadding;
}

}  // namespace

void LilygoRunner::Init() {}

void LilygoRunner::Draw(const eink::HAData &data, struct tm &now, int n_runs) {
  correct_adc_reference();

  int y = 0;

  time_t now_ts = eink::ToEpoch(now);

  y = DrawHeader(display, now, y);
  y = DrawWeather(display, data.weather, now_ts, y);
  y = DrawTempCO2(display, data.temp, data.co2, now_ts, y);
  y = DrawSoilMoistures(display, data.soil_moistures, now_ts, y);
  DrawFooter(display, n_runs);

  Serial.printf("Before updating %ld\n", millis() - now_ts);
  display.Update();
  Serial.printf("After updating %ld\n", millis() - now_ts);

  delay(2000);
}

}  // namespace lilygo
}  // namespace eink