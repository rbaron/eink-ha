#include "eink/display.h"

#include <Arduino.h>

#include "Firasans.h"
#include "opensans16.h"
#include "opensans24.h"

namespace eink {
namespace {

const EpdFont& GetFont(FontSize size) {
  switch (size) {
    case FontSize::Size12:
      return FiraSans_12;
    case FontSize::Size16:
      return OpenSans16;
    case FontSize::Size24:
      return OpenSans24;
  }
}

}  // namespace

Display::Display() {
  epd_init(EPD_OPTIONS_DEFAULT);
  hl_ = epd_hl_init(EPD_BUILTIN_WAVEFORM);
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
  fb_ = epd_hl_get_framebuffer(&hl_);
  epd_clear();
}

void Display::Clear() {
  epd_hl_set_all_white(&hl_);
}

void Display::DrawRect(int y, int x, int h, int w, uint8_t color) {
  EpdRect rect{x, y, w, h};
  epd_fill_rect(rect, color, fb_);
}

void Display::DrawText(int y, int x, const char* text, uint8_t color,
                       FontSize size) {
  return DrawText(y, x, text, color, size, DrawTextDirection::LTR);
}

void Display::DrawText(int y, int x, const char* text, uint8_t color,
                       FontSize size, DrawTextDirection dir) {
  EpdFontProperties font_props = epd_font_properties_default();
  if (dir == DrawTextDirection::RTL) {
    font_props.flags = EPD_DRAW_ALIGN_RIGHT;
  }
  font_props.fg_color = color;
  font_props.bg_color = 0xff;

  epd_write_string(&GetFont(size), text, &x, &y, fb_, &font_props);
}

void Display::Update() {
  epd_poweron();
  epd_hl_update_screen(&hl_, MODE_GC16, 20);
  epd_poweroff();
}

int Display::FontHeight(FontSize f) {
  return GetFont(f).advance_y;
}

}  // namespace eink