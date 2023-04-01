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

void scr_to_upper() {
  const auto script_value = game::Scr_GetConstString(0);
  const auto* string = game::SL_ConvertToString(script_value);

  char out[1024]{};
  bool changed = false;

  std::size_t i = 0;
  while (i < sizeof(out)) {
    const auto value = *string;
    const auto result =
        static_cast<char>(std::toupper(static_cast<unsigned char>(value)));

    out[i] = result;

    if (value != result) {
      changed = true;
    }

    if (result == '\0') {
      break;
    }

    ++string;
    ++i;
  }

  if (i >= sizeof(out)) {
    game::Scr_Error("string too long");
    return;
  }

  if (changed) {
    game::Scr_AddString(out);
  } else {
    game::SL_AddRefToString(script_value);
    game::Scr_AddConstString(script_value);
    game::SL_RemoveRefToString(script_value);
  }
}
} // namespace

class string final : public component_interface {
public:
  void post_load() override {
    add_function("StrICmp", scr_str_i_cmp);
    add_function("IsEndStr", scr_is_end_str);
    add_function("ToUpper", scr_to_upper);
  }
};
} // namespace gsc

REGISTER_COMPONENT(gsc::string)
