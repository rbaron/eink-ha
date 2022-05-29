#ifndef _EINK_RUNNER_H_
#define _EINK_RUNNER_H_

#include "eink/ha_client.h"

namespace eink {

class Runner {
 public:
  virtual void Init() = 0;
  virtual void Draw(const eink::HAData &data, struct tm &now, int n_runs) = 0;
};

}  // namespace eink
#endif  // _EINK_RUNNER_H_