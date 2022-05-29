#ifndef _EINK_LILYGO_RUNNER_H
#define _EINK_LILYGO_RUNNER_H

#include "eink/lilygo/eink_display.h"
#include "eink/runner.h"

namespace eink {
namespace lilygo {

class LilygoRunner : public eink::Runner {
 public:
  void Init() override;
  void Draw(const eink::HAData &data, struct tm &now, int n_runs) override;

 private:
  eink::lilygo::Display display;
};

}  // namespace lilygo
}  // namespace eink

#endif  ///_EINK_LILYGO_RUNNER_H