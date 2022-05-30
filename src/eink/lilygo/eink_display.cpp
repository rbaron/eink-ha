#ifdef EINK_LILYGO

#include "eink/lilygo/eink_display.h"

#include <Arduino.h>

#include "eink/lilygo/fonts/Firasans/Firasans.h"
#include "eink/lilygo/fonts/OpenSans/opensans16.h"
#include "eink/lilygo/fonts/OpenSans/opensans16b.h"
#include "eink/lilygo/fonts/OpenSans/opensans24.h"

namespace eink {
namespace lilygo {

namespace {

const EpdFont& GetFont(FontSize size) {
  switch (size) {
    case FontSize::Size12:
      return FiraSans_12;
    case FontSize::Size16:
      return OpenSans16;
    case FontSize::Size16b:
      return OpenSans16B;
    case FontSize::Size24:
      return OpenSans24;
  }
  Serial.printf("[eink_display] Unable to get font\n");
}

}  // namespace

Display::Display() {
  epd_init(EPD_OPTIONS_DEFAULT);
  hl_ = epd_hl_init(EPD_BUILTIN_WAVEFORM);
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
  // epd_set_rotation(EPD_ROT_PORTRAIT);
  fb_ = epd_hl_get_framebuffer(&hl_);
  epd_clear();
}

void Display::Clear() {
  epd_fullclear(&hl_, 25);
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
  epd_hl_update_screen(&hl_, MODE_GC16, 25);
  epd_poweroff();
}

int Display::FontHeight(FontSize f) {
  return GetFont(f).advance_y;
}

}  // namespace lilygo
}  // namespace eink

#endif  // EINK_LILYGO