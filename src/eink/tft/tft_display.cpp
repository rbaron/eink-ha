#include "eink/tft/tft_display.h"

#include <Arduino.h>

namespace eink {
namespace tft {
namespace {

constexpr int kMISOPin = 25;
constexpr int kMOSIPin = 26;
constexpr int kCLKPin = 27;
constexpr int kDCPin = 14;
constexpr int kCSPin = 12;
constexpr int kRSTPin = 15;

}  // namespace

TFTDisplay::TFTDisplay() {
  tft_ = std::unique_ptr<Adafruit_ILI9341>(new Adafruit_ILI9341(
      kCSPin, kDCPin, kMOSIPin, kCLKPin, kRSTPin, kMISOPin));
  tft_->begin();
}

void TFTDisplay::Clear() {
  tft_->fillScreen(ILI9341_WHITE);
  yield();
}

void TFTDisplay::DrawRect(int y, int x, int h, int w, uint16_t color) {
  tft_->drawRect(x, y, w, h, color);
}

void TFTDisplay::DrawText(int y, int x, const char* text, uint16_t color,
                          FontSize size) {
  return DrawText(y, x, text, color, size, DrawTextDirection::LTR);
}

void TFTDisplay::DrawText(int y, int x, const char* text, uint16_t color,
                          FontSize size, DrawTextDirection dir) {
  switch (size) {
    case FontSize::Size12:
      tft_->setTextSize(1);
      break;
    case FontSize::Size16b:
    case FontSize::Size16:
      tft_->setTextSize(2);
      break;
    case FontSize::Size24:
      tft_->setTextSize(3);
      break;
  }
  tft_->setCursor(x, y);
  tft_->setTextColor(color);
  tft_->println(text);
}

int TFTDisplay::FontHeight(FontSize f) {
  switch (f) {
    case FontSize::Size12:
      return 8;
    case FontSize::Size16:
    case FontSize::Size16b:
      return 16;
    case FontSize::Size24:
      return 24;
  }
}

}  // namespace tft
}  // namespace eink