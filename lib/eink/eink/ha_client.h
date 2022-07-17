#ifndef __EINK_HACLIENT_H__
#define __EINK_HACLIENT_H__

#include <WiFiClient.h>
#include <time.h>

#include <vector>

#define EINK_HACLIENT_WAIT_TIMEOUT_MS 10000

namespace eink {

struct SoilMoisture {
  std::string name;
  double value;
  time_t last_updated;
  std::string error;
};

struct Weather {
  double temp;
  std::string state;
  time_t last_updated;
};

struct Temp {
  double temp;
  time_t last_updated;
};

struct CO2 {
  int ppm;
  time_t last_updated;
};

struct HAData {
  std::vector<SoilMoisture> soil_moistures;
  Weather weather;
  CO2 co2;
  Temp temp;
};

class HAClient {
 public:
  HAClient(const char *url, const char *token) : url_(url), token_(token) {}
  HAData FetchData();

 private:
  const std::string url_;
  const std::string token_;
};
}  // namespace eink
#endif  // __EINK_HACLIENT_H__