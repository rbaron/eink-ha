#ifndef __EINK_HACLIENT_H__
#define __EINK_HACLIENT_H__

#include <WiFiClient.h>
#include <time.h>

#define EINK_HACLIENT_WAIT_TIMEOUT_MS 10000

namespace eink {

struct SoilMoisture {
  std::string name;
  double value;
  time_t last_updated;
  std::string error;
};

class HAClient {
 public:
  HAClient(const char *url, const char *token) : url_(url), token_(token) {}
  std::vector<SoilMoisture> FetchSoilMoisture();

 private:
  const std::string url_;
  const std::string token_;
};
}  // namespace eink
#endif  // __EINK_HACLIENT_H__