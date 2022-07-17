#ifndef __EINK_M5PAPER_RUNNER_H__
#define __EINK_M5PAPER_RUNNER_H__

#include "eink/m5paper/eink_display.h"
#include "eink/runner.h"

namespace eink {
namespace m5paper {

class M5PaperRunner : public eink::Runner {
 public:
  void Init() override;
  void Draw(const eink::HAData &data, struct tm &now, int n_runs) override;

 private:
  eink::m5paper::Display display_;
};

}  // namespace m5paper
}  // namespace eink

#endif  // __EINK_M5PAPER_RUNNER_H__