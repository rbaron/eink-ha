#include "eink/m5paper/eink_display.h"

namespace eink {
namespace m5paper {
namespace {

uint32_t GetM5Color(uint8_t c) {
  return UINT32_MAX - c * (UINT32_MAX / UINT8_MAX);
}

uint16_t GetM5FontSize(FontSize s) {
  switch (s) {
    case FontSize::Size12:
      return 2;
    case FontSize::Size16:
    case FontSize::Size16b:
      return 4;
    case FontSize::Size24:
      return 6;
    default:
      return 2;
  }
}

}  // namespace

Display::Display() : canvas_(&M5.EPD) {
  M5.begin();
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);
  M5.RTC.begin();
  canvas_.createCanvas(EINK_DISPLAY_HEIGHT, EINK_DISPLAY_WIDTH);
  canvas_.setTextSize(GetM5FontSize(FontSize::Size12));
}

void Display::Clear() {
  // M5.EPD.Clear(true);
  canvas_.clear();
}

void Display::DrawRect(int y, int x, int h, int w, uint8_t color) {
  canvas_.fillRect(x, y, w, h, GetM5Color(color));
  // canvas_.fillRect(x, y, w, h, 0xffff);
}

void Display::DrawText(int y, int x, const char* text, uint8_t color,
                       FontSize size) {
  return DrawText(y, x, text, color, size, DrawTextDirection::LTR);
}

void Display::DrawText(int y, int x, const char* text, uint8_t color,
                       FontSize size, DrawTextDirection dir) {
  return DrawText(y, x, text, color, size, dir, 0xff);
}

void Display::DrawText(int y, int x, const char* text, uint8_t color,
                       FontSize size, DrawTextDirection dir, uint8_t bg_color) {
  canvas_.setTextSize(GetM5FontSize(size));
  y -= FontHeight(size);
  if (dir == DrawTextDirection::RTL) {
    x -= canvas_.textWidth(text);
  }
  canvas_.setTextColor(GetM5Color(color), GetM5Color(bg_color));
  canvas_.drawString(text, x, y);
}

void Display::Update() {
  canvas_.pushCanvas(0, 0, UPDATE_MODE_GC16);
}

int Display::FontHeight(FontSize f) {
  canvas_.setTextSize(GetM5FontSize(f));
  return canvas_.fontHeight();
}

}  // namespace m5paper
}  // namespace eink