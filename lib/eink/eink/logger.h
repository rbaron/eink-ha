#ifndef __EINK_LOGGER__
#define __EINK_LOGGER__

#include "epd_driver.h"
#include "epd_highlevel.h"

#define EINK_DISPLAY_WIDTH EPD_WIDTH
#define EINK_DISPLAY_HEIGHT EPD_HEIGHT

#define LOG(...) Logger::Get().Printf(__VA_ARGS__)

namespace eink {

// Singleton logging facility.
class Logger {
 public:
  static Logger& Get() {
    static Logger logger;
    return logger;
  }

 private:
  Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

 public:
  void Printf(const char* format, ...) const;
};

}  // namespace eink
#endif  // __EINK_LOGGER__