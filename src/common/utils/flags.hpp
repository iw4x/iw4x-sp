#pragma once
#include <optional>

namespace utils::flags {
bool has_flag(const std::string& flag);
}

#define IS_FLAG_ENABLED(flag)                                                  \
  static std::optional<bool> flag;                                             \
  if (!(flag).has_value()) {                                                   \
    (flag).emplace(utils::flags::has_flag(#flag));                             \
  }                                                                            \
  return (flag).value();
