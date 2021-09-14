#include "eink/time.h"

#include <Arduino.h>

#include <sstream>

#include "eink/logger.h"

namespace eink {
namespace {
constexpr long kGMTOffsetS = 3600;
constexpr long kDayLightOffsetS = 3600;

constexpr int kMinS = 60;
constexpr int kHourS = 60 * kMinS;
constexpr int kDayS = 24 * kHourS;
constexpr int kWeekS = 7 * kDayS;
constexpr int kYear = 365 * kDayS;
}  // namespace

void ConfigNTP() {
  configTime(kGMTOffsetS, kDayLightOffsetS, "pool.ntp.org");
}

struct tm GetCurrentTime() {
  struct tm t;
  int res = getLocalTime(&t);
  return t;
}

time_t ExtractTimezoneOffset(struct tm &t) {
  char buf[16];
  strftime(buf, sizeof(buf), "%z", &t);
  int offset = std::atoi(buf) / 100;
  return offset;
}

struct tm ParseISODate(const char *in) {
  std::string input(in);
  size_t dotpos = input.find(".");
  size_t colonpos = input.find(":", dotpos);

  // Construct a string like 2021-09-14T20:30+2000.
  std::string s = input.substr(0, dotpos) +
                  input.substr(dotpos + 7, colonpos - (dotpos + 7)) +
                  input.substr(colonpos + 1);
  struct tm t;
  const char *fmt = "%Y-%m-%dT%H:%M:%S%z";

  char *res = strptime(s.c_str(), fmt, &t);

  // This is a terrible hack and I hate it but I can't look at this code
  // anymore.
  // The reason is that the strptime call above is not working as
  // expected. I would expect the call above to parse like the following: Input:
  // 2021-09-14T08:12:53+0000 Output: Tuesday Sep 14 08:12 +0000 or Output:
  // Tuesday Sep 14 10:12 +0200 Instead, it's working like this: Input:
  // 2021-09-14T08:12:53+0000 Output: Tuesday Sep 14 08:12 +0200
  t.tm_hour += ExtractTimezoneOffset(t);
  // Updates the fields of t and propagates our hour increment.
  mktime(&t);
  return t;
}

std::string FormatTime(const struct tm &t) {
  char buf[64];
  strftime(buf, sizeof(buf), "%A %b %d %H:%M", &t);
  return buf;
}

std::string ToHumanDiff(time_t delta) {
  std::ostringstream out;
  if (delta < kMinS) {
    out << delta << " s";
  } else if (delta < kHourS) {
    out << delta / kMinS << " m";
  } else if (delta < kDayS) {
    out << delta / kHourS << " h";
  } else if (delta < kWeekS) {
    out << delta / kDayS << " d";
  } else if (delta < kYear) {
    out << delta / kWeekS << " w";
  } else {
    out << delta / kYear << " y";
  }
  return out.str();
}

time_t ToEpoch(struct tm &tm) {
  return mktime(&tm);
}

}  // namespace eink