#ifndef _EINK_TFT_RUNNER_H_
#define _EINK_TFT_RUNNER_H_

#include "eink/runner.h"
#include "eink/tft/tft_display.h"

namespace eink {
namespace tft {

class TFTRunner : public eink::Runner {
 public:
  void Init() override;
  void Draw(const eink::HAData &data, struct tm &now, int n_runs) override;

 private:
  eink::tft::TFTDisplay display_;
};

}  // namespace tft
}  // namespace eink

#endif  ///_EINK_TFT_RUNNER_H_