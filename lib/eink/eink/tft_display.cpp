#ifdef EINK_TFT

#include "eink/tft_display.h"

#include <Arduino.h>

namespace eink {
namespace {}  // namespace

TFTDisplay::Display() {}

void TFTDisplay::Clear() {}

void TFTDisplay::DrawRect(int y, int x, int h, int w, uint8_t color) {}

void TFTDisplay::DrawText(int y, int x, const char* text, uint8_t color,
                          FontSize size) {
  return DrawText(y, x, text, color, size, DrawTextDirection::LTR);
}

void TFTDisplay::DrawText(int y, int x, const char* text, uint8_t color,
                          FontSize size, DrawTextDirection dir) {}

void TFTDisplay::Update() {}

int TFTDisplay::FontHeight(FontSize f) {}

}  // namespace eink

#endif  // EINK_TFT