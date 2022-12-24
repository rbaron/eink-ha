#include "eink/ha_client.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <string>

#include "eink/time.h"

#define EINK_HA_CLIENT_HTTP_RESP_SIZE 100000

namespace eink {
namespace {

SoilMoisture ParseSoilMoisture(const JsonObject& entity) {
  SoilMoisture s;
  s.name = entity["attributes"]["friendly_name"].as<std::string>();
  s.name = s.name.substr(0, s.name.find(" Soil"));
  s.name = s.name.substr(0, s.name.find(" Moisture"));

  auto state = entity["state"];
  if (state.as<std::string>() == "nan" ||
      state.as<std::string>() == "unknown" ||
      state.as<std::string>() == "unavailable") {
    s.error = state.as<std::string>();
  } else {
    s.value = entity["state"].as<double>();
  }

  // Dummy call to fix some weird bug.
  struct tm last_updated =
      ParseISODate(entity["last_updated"].as<const char*>());
  last_updated = ParseISODate(entity["last_updated"].as<const char*>());
  Serial.printf("%s: %f (error: %s), last updated: %s (parsed from: %s)\n",
                s.name.c_str(), s.value, s.error.c_str(),
                FormatTime(last_updated).c_str(),
                entity["last_updated"].as<const char*>());
  s.last_updated = ToEpoch(last_updated);
  return s;
}

void ParseWeatherData(const JsonObject& entity, Weather& w) {
  const std::string name = entity["entity_id"].as<std::string>();
  if (name == "sensor.openweathermapzurich_temperature") {
    w.temp = entity["state"].as<double>();
    struct tm last_updated =
        ParseISODate(entity["last_updated"].as<const char*>());
    w.last_updated = ToEpoch(last_updated);
    Serial.printf("Weather temp: %f\n", w.temp);
  } else if (name == "sensor.openweathermapzurich_condition") {
    w.state = entity["state"].as<std::string>();
    w.state[0] = std::toupper(w.state[0]);
    Serial.printf("Weather state: %s\n", w.state.c_str());
  }
}

void ParseCO2Data(const JsonObject& entity, CO2& res) {
  res.ppm = entity["state"].as<int>();
  struct tm last_updated =
      ParseISODate(entity["last_updated"].as<const char*>());
  res.last_updated = ToEpoch(last_updated);
  Serial.printf("CO2: %d, updated %s ago\n", res.ppm,
                FormatTime(last_updated).c_str());
}

void ParseTempData(const JsonObject& entity, Temp& res) {
  res.temp = entity["state"].as<double>();
  struct tm last_updated =
      ParseISODate(entity["last_updated"].as<const char*>());
  res.last_updated = ToEpoch(last_updated);
  Serial.printf("Temp: %f, updated %s ago\n", res.temp,
                FormatTime(last_updated).c_str());
}

void ParseSolarLEDsData(const JsonObject& entity, SolarLEDs& s) {
  const std::string name = entity["entity_id"].as<std::string>();
  if (name == "sensor.solarleds_bus_voltage") {
    s.voltage = entity["state"].as<float>();
    struct tm last_updated =
        ParseISODate(entity["last_updated"].as<const char*>());
    s.last_updated = ToEpoch(last_updated);
    Serial.printf("Solarleds voltage: %f\n", s.voltage);
  } else if (name == "sensor.solarleds_current") {
    s.current = entity["state"].as<float>();
    Serial.printf("Solarleds current: %f\n", s.current);
  }
}

}  // namespace

void HAClient::FetchData(HAData& data) {
  HTTPClient http;
  http.useHTTP10(true);
  std::string url = url_ + "/states";
  http.begin(url.c_str());
  std::string header = std::string("Bearer ") + token_;
  http.addHeader("Authorization", header.c_str());
  int code = http.GET();

  Serial.printf("HTTP status code: %d\n", code);

  DynamicJsonDocument doc(EINK_HA_CLIENT_HTTP_RESP_SIZE);
  deserializeJson(doc, http.getStream());
  Serial.printf("JSON: %d\n", doc.size());

  for (const auto& el : doc.as<JsonArray>()) {
    std::string entity_id(el["entity_id"].as<std::string>());

    if (entity_id.find("moisture") != entity_id.npos &&
        entity_id.find("test") == entity_id.npos) {
      data.soil_moistures.push_back(ParseSoilMoisture(el.as<JsonObject>()));
    } else if (entity_id.find("sensor.openweathermapzurich") !=
               entity_id.npos) {
      ParseWeatherData(el.as<JsonObject>(), data.weather);
    } else if (entity_id == "sensor.mh_z19_co2_value") {
      ParseCO2Data(el.as<JsonObject>(), data.co2);
    } else if (entity_id == "sensor.atc_living_room_temperature") {
      ParseTempData(el.as<JsonObject>(), data.temp);
    } else if (entity_id.find("sensor.solarleds") != entity_id.npos) {
      ParseSolarLEDsData(el.as<JsonObject>(), data.solarleds);
    }
  }
  std::sort(data.soil_moistures.begin(), data.soil_moistures.end(),
            [](const SoilMoisture& lhs, const SoilMoisture& rhs) {
              // If lhs has an error, let rhs be less, so it will rank earlier.
              if (lhs.error != "" && rhs.error == "") {
                return false;
              } else if (lhs.error == "" && rhs.error != "") {
                return true;
              }
              return lhs.value < rhs.value;
            });
}

}  // namespace eink