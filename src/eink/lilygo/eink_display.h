#ifdef EINK_LILYGO

#ifndef __EINK_DISPLAY__
#define __EINK_DISPLAY__

#include "eink/display_utils.h"
#include "epd_driver.h"
#include "epd_highlevel.h"

#define EINK_DISPLAY_WIDTH EPD_WIDTH
#define EINK_DISPLAY_HEIGHT EPD_HEIGHT

namespace eink {
namespace lilygo {

class Display {
 public:
  Display();
  Display(const Display&) = delete;
  Display& operator=(const Display&) = delete;

  void Clear();
  void DrawRect(int y, int x, int h, int w, uint8_t color);
  void DrawText(int y, int x, const char* text, uint8_t color, FontSize size);
  void DrawText(int y, int x, const char* text, uint8_t color, FontSize size,
                DrawTextDirection dir);
  void Update();
  int FontHeight(FontSize f);

 private:
  // High level display state.
  EpdiyHighlevelState hl_;
  // Framebuffer.
  uint8_t* fb_;
};

}  // namespace lilygo
}  // namespace eink

#endif  // __EINK_DISPLAY__

#endif  // EINK_LILIYGO