#ifndef __EINK_M5PAPER_DISPLAY__
#define __EINK_M5PAPER_DISPLAY__

#include <M5EPD.h>

#include "eink/display_utils.h"

#define EINK_DISPLAY_HEIGHT 540
#define EINK_DISPLAY_WIDTH 960

namespace eink {
namespace m5paper {

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
  void DrawText(int y, int x, const char* text, uint8_t color, FontSize size,
                DrawTextDirection dir, uint8_t bg_color);
  void Update();
  int FontHeight(FontSize f);

 private:
  M5EPD_Canvas canvas_;
};

}  // namespace m5paper
}  // namespace eink

#endif  // __EINK_M5PAPER_DISPLAY__