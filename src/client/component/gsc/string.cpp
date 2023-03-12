#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "extension.hpp"

namespace gsc {
namespace {
void scr_str_i_cmp() {
  const auto* string1 = game::SL_ConvertToString(game::Scr_GetConstString(0));
  const auto* string2 = game::SL_ConvertToString(game::Scr_GetConstString(1));

  game::Scr_AddInt(game::I_stricmp(string1, string2));
}

void scr_is_end_str() {
  const auto* str = game::Scr_GetString(0);
  const auto* suffix = game::Scr_GetString(1);

  const auto str_len = std::strlen(str);
  const auto suffix_len = std::strlen(suffix);

  if (suffix_len > str_len) {
    game::Scr_AddInt(0);
    return;
  }

  game::Scr_AddInt(
      std::memcmp(str + str_len - suffix_len, suffix, suffix_len) == 0);
}
} // namespace

class string final : public component_interface {
public:
  void post_load() override {
    add_function("StrICmp", scr_str_i_cmp);
    add_function("IsEndStr", scr_is_end_str);
  }
};
} // namespace gsc

REGISTER_COMPONENT(gsc::string)
