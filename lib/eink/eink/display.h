#ifndef __EINK_DISPLAY__
#define __EINK_DISPLAY__

#include "epd_driver.h"
#include "epd_highlevel.h"

#define EINK_DISPLAY_WIDTH EPD_WIDTH
#define EINK_DISPLAY_HEIGHT EPD_HEIGHT

namespace eink {

enum class FontSize {
  Size12,
};

enum class DrawTextDirection {
  LTR,
  RTL,
};

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

 private:
  // High level display state.
  EpdiyHighlevelState hl_;
  // Framebuffer.
  uint8_t* fb_;
};

}  // namespace eink
#endif  // __EINK_DISPLAY__