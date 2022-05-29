#ifdef EINK_TFT

#ifndef __EINK_TFT_DISPLAY__
#define __EINK_TFT_DISPLAY__

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>

#include <memory>

#include "eink/display.h"

#define TFT_HEIGHT 320
#define TFT_WIDTH 240
#define EINK_DISPLAY_HEIGHT TFT_HEIGHT
#define EINK_DISPLAY_WIDTH TFT_WIDTH

#define TFT_MISO 25
#define TFT_MOSI 26
#define TFT_CLK 27
#define TFT_DC 14
#define TFT_CS 12
#define TFT_RST 15

namespace eink {

class TFTDisplay {
 public:
  using Color = uint8_t;

  TFTDisplay();
  TFTDisplay(const TFTDisplay&) = delete;
  TFTDisplay& operator=(const TFTDisplay&) = delete;

  void Clear();
  void DrawRect(int y, int x, int h, int w, uint8_t color);
  void DrawText(int y, int x, const char* text, uint8_t color, FontSize size);
  void DrawText(int y, int x, const char* text, uint8_t color, FontSize size,
                DrawTextDirection dir);
  void Update();
  int FontHeight(FontSize f);

 private:
  // std::unique_ptr<Adafruit_ILI
  std::unique_ptr<Adafruit_ILI9341> tft_;
};

}  // namespace eink
#endif  // __EINK_TFT_DISPLAY__

#endif  // EINK_TFT