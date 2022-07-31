#include <std_include.hpp>

#include <utils/nt.hpp>

#include "game_module.hpp"

namespace game_module {
utils::nt::library get_host_module() {
  static utils::nt::library host{};
  return host;
}
} // namespace game_module
