#ifndef __EINK_TFT_DISPLAY__
#define __EINK_TFT_DISPLAY__

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>

#include <memory>

#include "eink/display_utils.h"

namespace eink {
namespace tft {

class TFTDisplay {
 public:
  using Color = uint8_t;

  TFTDisplay();
  TFTDisplay(const TFTDisplay&) = delete;
  TFTDisplay& operator=(const TFTDisplay&) = delete;

  void Clear();
  void DrawRect(int y, int x, int h, int w, uint16_t color);
  void DrawText(int y, int x, const char* text, uint16_t color, FontSize size);
  void DrawText(int y, int x, const char* text, uint16_t color, FontSize size,
                DrawTextDirection dir);
  int FontHeight(FontSize f);

 private:
  std::unique_ptr<Adafruit_ILI9341> tft_;
};

}  // namespace tft
}  // namespace eink
#endif  // __EINK_TFT_DISPLAY__