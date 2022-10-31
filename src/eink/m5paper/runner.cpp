#include "eink/m5paper/runner.h"

#include <M5EPD.h>

#include "eink/display_utils.h"
#include "eink/time.h"
#include "esp_adc_cal.h"

#define BATT_PIN 36

// Consider a sensor value stale if its older than this value.
#define STALE_THRESHOLD_S 2 * 3600
#define STALE_COLOR 200

#define SOIL_MOISTURE_ROWS 9
#define SOIL_MOISTURE_COLS 2

namespace eink {
namespace m5paper {
namespace {

constexpr int kPadding = 10;

int DrawHeader(eink::m5paper::Display &display, const struct tm &now, int y) {
  int text_size = display.FontHeight(eink::FontSize::Size12);

  std::string tstring = eink::FormatTime(now);
  y += text_size;
  display.DrawText(y, kPadding, tstring.c_str(), 0, eink::FontSize::Size12,
                   eink::DrawTextDirection::LTR);
  return y;
}

void DrawFooter(eink::m5paper::Display &display, int n_runs) {
  std::string txt = "Run #" + to_fixed_str(n_runs);
  display.DrawText(EINK_DISPLAY_WIDTH - kPadding,
                   EINK_DISPLAY_HEIGHT - kPadding, txt.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
}

int DrawWeather(eink::m5paper::Display &display, const eink::Weather &w,
                time_t now, int y) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int weather_size = display.FontHeight(eink::FontSize::Size24);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y + header_size + kPadding, kPadding, "Weather", 0,
                   eink::FontSize::Size16b);
  y += header_size + kPadding;

  int weather_y = y + 2 * kPadding + weather_size - kPadding;
  std::string val = to_fixed_str(w.temp, 1) + " C";
  display.DrawText(weather_y, kPadding, val.c_str(), 0, eink::FontSize::Size24);

  int state_y = y + 2 * kPadding + text_size;
  display.DrawText(state_y, EINK_DISPLAY_HEIGHT - kPadding, w.state.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  display.DrawText(state_y + kPadding + text_size,
                   EINK_DISPLAY_HEIGHT - kPadding,
                   (eink::ToHumanDiff(now - w.last_updated) + " ago").c_str(),
                   0, eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  return weather_y;
}

int DrawTempCO2(eink::m5paper::Display &display, const eink::Temp &t,
                const eink::CO2 &w, time_t now, int y) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int weather_size = display.FontHeight(eink::FontSize::Size24);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y + header_size + kPadding, kPadding, "Living Room", 0,
                   eink::FontSize::Size16b);
  y += header_size + kPadding;

  y += 2 * kPadding + weather_size - kPadding;
  std::string val = to_fixed_str(t.temp, 1) + " C";
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

int DrawSoilMoistures(eink::m5paper::Display &display,
                      std::vector<eink::SoilMoisture> soil_moistures,
                      time_t now, int y0) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y0 + header_size + 1 * kPadding, kPadding, "b-parasites", 0,
                   eink::FontSize::Size16b);
  y0 += header_size + 2 * kPadding;

  const uint8_t bg0 = 0xff, bg1 = 254;

  int y_rows = y0;
  int row_height = 2 * kPadding + text_size;
  for (int i = 0; i < SOIL_MOISTURE_ROWS; i++) {
    display.DrawRect(y0 + i * row_height, 0, row_height, EINK_DISPLAY_HEIGHT,
                     i % 2 ? bg0 : bg1);
    y_rows += row_height;
  }

  for (int i = 0; i < soil_moistures.size(); i++) {
    const eink::SoilMoisture &s = soil_moistures[i];

    int col = i / SOIL_MOISTURE_ROWS;
    int row = i % SOIL_MOISTURE_ROWS;

    int x = kPadding + col * EINK_DISPLAY_HEIGHT / SOIL_MOISTURE_COLS;
    int y = y0 + kPadding + text_size + row * row_height;

    uint8_t bg_color = row % 2 ? bg0 : bg1;
    Serial.printf("%s - diff: %d (last_updated: %d), %s\n", s.name.c_str(),
                  now - s.last_updated, s.last_updated,
                  ToHumanDiff(now - s.last_updated).c_str());
    uint8_t text_color = now - s.last_updated > 2 * 3600 ? STALE_COLOR : 0;
    display.DrawText(y, x, s.name.c_str(), text_color, eink::FontSize::Size12,
                     eink::DrawTextDirection::LTR, bg_color);

    std::string val = s.error == "" ? (to_fixed_str(s.value, 0) + "%") : "?";
    display.DrawText(
        y, x - 2 * kPadding + EINK_DISPLAY_HEIGHT / SOIL_MOISTURE_COLS,
        val.c_str(), text_color, eink::FontSize::Size12,
        eink::DrawTextDirection::RTL, bg_color);
  }

  return y_rows + kPadding;
}

int DrawSolarLEDs(eink::m5paper::Display &display, const SolarLEDs &s,
                  time_t now, int y0) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y0 + header_size, kPadding, "Balcony Lights", 0,
                   eink::FontSize::Size16b);
  y0 += header_size + kPadding;
  // y0 += kPadding;

  const uint8_t bg = 254;

  int row_height = 2 * kPadding + text_size;
  display.DrawRect(y0, 0, row_height, EINK_DISPLAY_HEIGHT, bg);

  uint8_t text_color = now - s.last_updated > 2 * 3600 ? STALE_COLOR : 0;

  int y = y0 + kPadding + text_size;
  std::string voltage = to_fixed_str(s.voltage, 2) + " V";
  display.DrawText(y, kPadding, voltage.c_str(), text_color,
                   eink::FontSize::Size12, eink::DrawTextDirection::LTR, bg);
  std::string current = to_fixed_str(s.current, 2) + " mA";
  display.DrawText(y, EINK_DISPLAY_HEIGHT / 2, current.c_str(), text_color,
                   eink::FontSize::Size12, eink::DrawTextDirection::LTR, bg);
  return y0 + row_height;
}

}  // namespace

void M5PaperRunner::Init() {
  // Serial.println("[M5PaperRunner::Init()] Will init display");
  display_.Clear();
  display_.DrawText(kPadding, kPadding, "Fetching data...", 0,
                    eink::FontSize::Size12);
  display_.Update();
}

void M5PaperRunner::Draw(const eink::HAData &data, struct tm &now, int n_runs) {
  int y = kPadding;

  time_t now_ts = eink::ToEpoch(now);

  display_.Clear();
  y = DrawHeader(display_, now, y);
  y = DrawWeather(display_, data.weather, now_ts, y);
  y = DrawTempCO2(display_, data.temp, data.co2, now_ts, y);
  y = DrawSoilMoistures(display_, data.soil_moistures, now_ts, y);
  y = DrawSolarLEDs(display_, data.solarleds, now_ts, y);
  DrawFooter(display_, n_runs);

  Serial.printf("Before updating %ld\n", millis() - now_ts);
  display_.Update();
  Serial.printf("After updating %ld\n", millis() - now_ts);

  delay(5000);
}

}  // namespace m5paper
}  // namespace eink