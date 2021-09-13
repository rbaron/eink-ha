#include "eink/logger.h"

#include <Arduino.h>

#include <cstdarg>

#define EINK_PRINTF_BUFSIZ 256

namespace eink {

Logger::Logger() {
  Serial.begin(115200);
}

// TODO: handle case when buffer is too small. For now, trunctate it.
void Logger::Printf(const char* format, ...) const {
  char buf[EINK_PRINTF_BUFSIZ];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  Serial.print(buf);
}

}  // namespace eink