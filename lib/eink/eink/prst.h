#ifndef __EINK_PRST_H__
#define __EINK_PRST_H__

#include <PubSubClient.h>
#include <WiFiClient.h>

#define EINK_PRST_WAIT_TIMEOUT_MS 6000

namespace eink {

class PRST {
 public:
  PRST();
  boolean Connect(const char *mqtt_host, const char *mqtt_user,
                  const char *mqtt_pass);
  boolean SubscribeToAll();
  boolean WaitAllMessages();

 private:
  WiFiClient wfcli_;
  PubSubClient pubsub_;
};
}  // namespace eink
#endif  // __EINK_PRST_H__