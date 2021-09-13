#include "eink/display.h"

#include "Firasans.h"

namespace eink {

Display::Display() {
  epd_init(EPD_OPTIONS_DEFAULT);
  hl_ = epd_hl_init(EPD_BUILTIN_WAVEFORM);
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
  fb_ = epd_hl_get_framebuffer(&hl_);
  epd_clear();
}

void Display::DrawRect(int y, int x, int h, int w, uint8_t color) {
  EpdRect rect{x, y, w, h};
  epd_fill_rect(rect, color, fb_);
}

void Display::DrawText(int y, int x, const char* text, uint8_t color,
                       FontSize size) {
  if (size == FontSize::Size12) {
    epd_write_default(&FiraSans_12, text, &x, &y, fb_);
  }
}

void Display::Update() {
  epd_poweron();
  epd_hl_update_screen(&hl_, MODE_GC16, 25);
  epd_poweroff();
}

}  // namespace eink