#include "eink/ha_client.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <string>

#include "eink/logger.h"
#include "eink/time.h"

#define EINK_HA_CLIENT_HTTP_RESP_SIZE 100000

namespace eink {
namespace {

SoilMoisture ParseSoilMoisture(const JsonObject& entity) {
  SoilMoisture s;
  s.name = entity["attributes"]["friendly_name"].as<std::string>();
  s.name = s.name.substr(0, s.name.find(" Soil"));

  auto state = entity["state"];
  if (state.as<std::string>() == "nan" ||
      state.as<std::string>() == "unknown") {
    s.error = state.as<std::string>();
  } else {
    s.value = entity["state"].as<double>();
  }

  struct tm last_updated =
      ParseISODate(entity["last_updated"].as<const char*>());
  LOG("%s: %f (error: %s), last updated: %s\n", s.name.c_str(), s.value,
      s.error.c_str(), FormatTime(last_updated).c_str());
  s.last_updated = ToEpoch(last_updated);
  return s;
}
}  // namespace

std::vector<SoilMoisture> HAClient::FetchSoilMoisture() {
  HTTPClient http;
  http.useHTTP10(true);
  std::string url = url_ + "/states";
  http.begin(url.c_str());
  std::string header = std::string("Bearer ") + token_;
  http.addHeader("Authorization", header.c_str());
  int code = http.GET();

  LOG("HTTP status code: %d\n", code);

  DynamicJsonDocument doc(EINK_HA_CLIENT_HTTP_RESP_SIZE);
  deserializeJson(doc, http.getStream());
  LOG("JSON: %d\n", doc.size());

  std::vector<SoilMoisture> res;
  for (const auto& el : doc.as<JsonArray>()) {
    std::string entity_id(el["entity_id"].as<std::string>());

    if (entity_id.find("soil_moisture") != entity_id.npos &&
        entity_id.find("test") == entity_id.npos) {
      // LOG("Entity ID: %s\n", entity_id.c_str());
      res.push_back(ParseSoilMoisture(el.as<JsonObject>()));
    }
  }
  std::sort(res.begin(), res.end(),
            [](const SoilMoisture& lhs, const SoilMoisture& rhs) {
              return lhs.value < rhs.value;
            });
  return res;
}

}  // namespace eink