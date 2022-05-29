#ifndef _EINK_DISPLAY_UTILS_H_
#define _EINK_DISPLAY_UTILS_H_

#include <sstream>
#include <string>

namespace eink {

enum class FontSize {
  Size12,
  Size16,
  Size16b,
  Size24,
};

enum class DrawTextDirection {
  LTR,
  RTL,
};

template <typename T>
std::string to_fixed_str(const T value, const int n = 0) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << value;
  return out.str();
}

}  // namespace eink

#endif  // _EINK_DISPLAY_UTILS_H_