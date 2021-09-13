#include "eink/prst.h"

#include <string>

#include "eink/logger.h"

namespace eink {
namespace {

static int nParasiteFound = 0;

// Callback for MQTT topics.
void mqttCallback(const char *topic, uint8_t *payload, unsigned int length) {
  const auto &logger = Logger::Get();
  // if (subs)
  std::string t(topic);
  if (t.find("soil_moisture") == t.npos) {
    return;
  }
  nParasiteFound++;
  logger.Printf("Found another parasite: %d (%s))\n", nParasiteFound, topic);
  // std::string p((char *)payload, length);
  // logger.Printf("Got payload from %s\n", topic);
  // logger.Printf("Payload: %s\n", p.c_str());
}
}  // namespace

PRST::PRST() {}

boolean PRST::Connect(const char *mqtt_host, const char *mqtt_user,
                      const char *mqtt_pass) {
  const auto &logger = Logger::Get();

  pubsub_.setClient(wfcli_);
  pubsub_.setServer(mqtt_host, 1883);
  pubsub_.setBufferSize(1024);
  pubsub_.setCallback(mqttCallback);

  bool connected = pubsub_.connect("eink", mqtt_user, mqtt_pass);
  logger.Printf("MQTT connected: %d\n", connected);

  pubsub_.publish("test", "test");
  return connected;
}

boolean PRST::SubscribeToAll() {
  boolean res = pubsub_.subscribe("homeassistant/sensor/co2sensors/#");
  if (!res) {
    Logger::Get().Printf("Error subscribing");
    return false;
  }
  res = pubsub_.subscribe("homeassistant/sensor/hall_bridge/#");
  if (!res) {
    Logger::Get().Printf("Error subscribing");
    return false;
  }
  return true;
}

boolean PRST::WaitAllMessages() {
  auto &logger = Logger::Get();
  unsigned int t0 = millis();
  while (millis() - t0 < EINK_PRST_WAIT_TIMEOUT_MS) {
    pubsub_.publish("test", "loop");
    pubsub_.loop();
    delay(50);
  }

  logger.Printf("Timeout waiting for PRST msgs\n.");
  return false;
}

}  // namespace eink