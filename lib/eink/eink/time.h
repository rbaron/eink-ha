#ifndef __EINK_TIME_H__
#define __EINK_TIME_H__

#include <time.h>

#include <string>

namespace eink {

void ConfigNTP();

struct tm GetCurrentTime();

// Simplified parsing of an ISO 8601 date string.
// Timezones and milliseconds are ignored.
struct tm ParseISODate(const char *iso);

std::string FormatTime(const struct tm &t);

std::string ToHumanDiff(time_t delta_s);

time_t ToEpoch(struct tm &tm);

}  // namespace eink
#endif  //__EINK_TIME_H__