#include "eink/tft/runner.h"

#include "eink/time.h"

namespace eink {
namespace tft {
namespace {
constexpr int kTFTHeight = 320;
constexpr int kTFTWidth = 240;
constexpr int kPadding = 8;
constexpr int kSoilMoistureRows = 7;
constexpr int kSoilMoistureCols = 2;

int DrawHeader(eink::tft::TFTDisplay &display, const struct tm &now, int y) {
  int text_size = display.FontHeight(eink::FontSize::Size12);

  std::string tstring = eink::FormatTime(now);
  Serial.printf("[TFTRunner] tstring: %s\n", tstring.c_str());
  y += text_size;
  display.DrawText(y, kPadding, tstring.c_str(), ILI9341_OLIVE,
                   eink::FontSize::Size12, eink::DrawTextDirection::LTR);
  return y + text_size + kPadding;
}

int DrawWeather(eink::tft::TFTDisplay &display, const eink::Weather &w,
                time_t now, int y) {
  int header_size = display.FontHeight(eink::FontSize::Size16b);
  int weather_size = display.FontHeight(eink::FontSize::Size24);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y, kPadding, "Weather", ILI9341_DARKGREY,
                   eink::FontSize::Size16b);
  y += header_size + kPadding;

  std::string val = to_fixed_str(w.temp, 1) + " C";
  display.DrawText(y, kPadding, val.c_str(), 0, eink::FontSize::Size24);

  display.DrawText(y, 2 * kTFTWidth / 3 + kPadding, w.state.c_str(), 0,
                   eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  display.DrawText(y + kPadding + text_size, 2 * kTFTWidth / 3 + kPadding,
                   (eink::ToHumanDiff(now - w.last_updated) + " ago").c_str(),
                   0, eink::FontSize::Size12, eink::DrawTextDirection::RTL);
  return y + weather_size + kPadding;
}

int DrawTempCO2(eink::tft::TFTDisplay &display, const eink::Temp &t,
                const eink::CO2 &w, time_t now, int y) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int weather_size = display.FontHeight(eink::FontSize::Size24);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y, kPadding, "Living Room", ILI9341_DARKGREY,
                   eink::FontSize::Size16b);
  y += header_size + kPadding;

  std::string val = to_fixed_str(t.temp, 1) + " C";
  display.DrawText(y, kPadding, val.c_str(), 0, eink::FontSize::Size24);

  display.DrawText(y, 2 * kTFTWidth / 3 + kPadding,
                   (eink::ToHumanDiff(now - t.last_updated) + " ago").c_str(),
                   0, eink::FontSize::Size12, eink::DrawTextDirection::RTL);

  return y + weather_size + kPadding;
}

int DrawSoilMoistures(eink::tft::TFTDisplay &display,
                      std::vector<eink::SoilMoisture> soil_moistures,
                      time_t now, int y0) {
  int header_size = display.FontHeight(eink::FontSize::Size16);
  int text_size = display.FontHeight(eink::FontSize::Size12);

  display.DrawText(y0, kPadding, "b-parasites", ILI9341_DARKGREY,
                   eink::FontSize::Size16b);
  y0 += header_size + kPadding;

  int y_rows = y0;
  for (int i = 0; i < kSoilMoistureRows; i++) {
    y_rows += kPadding + text_size;
  }

  for (int i = 0; i < soil_moistures.size(); i++) {
    const eink::SoilMoisture &s = soil_moistures[i];

    int col = i / kSoilMoistureRows;
    int row = i % kSoilMoistureRows;

    int x = kPadding + col * kTFTWidth / kSoilMoistureCols;
    int y = y0 + kPadding + row * (kPadding + text_size);

    uint16_t text_color = now - s.last_updated > 2 * 3600
                              ? ILI9341_DARKGREY
                              : (s.value < 30.0 ? ILI9341_RED : ILI9341_OLIVE);
    display.DrawText(y, x, s.name.c_str(), text_color, eink::FontSize::Size12);

    std::string val = s.error == "" ? (to_fixed_str(s.value, 0) + "%") : "?";
    display.DrawText(
        y, x - 2 * kPadding + kTFTWidth / kSoilMoistureCols - 6 * val.length(),
        val.c_str(), text_color, eink::FontSize::Size12,
        eink::DrawTextDirection::RTL);
  }

  return y_rows;
}

}  // namespace

void TFTRunner::Init() {}

void TFTRunner::Draw(const eink::HAData &data, struct tm &now, int n_runs) {
  time_t now_ts = eink::ToEpoch(now);

  display_.Clear();
  int y = 0;
  y = DrawHeader(display_, now, y);
  y = DrawWeather(display_, data.weather, now_ts, y);
  y = DrawTempCO2(display_, data.temp, data.co2, now_ts, y);
  y = DrawSoilMoistures(display_, data.soil_moistures, now_ts, y);
  yield();
}
}  // namespace tft
}  // namespace eink