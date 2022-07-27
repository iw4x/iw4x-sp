#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

#include <version.h>

namespace branding {
namespace {
const char* get_build_number() {
  return SHORTVERSION " latest " __DATE__ " " __TIME__;
}

const char* get_version_string() {
#ifdef _DEBUG
  const auto* build_type = "IW4_DEV SP";
#else
  const auto* build_type = "IW4x SP";
#endif

  const auto* result = utils::string::va(
      "%s %s build %s %s", build_type, "(Alpha)", get_build_number(),
      reinterpret_cast<const char*>(0x6A1574));

  return result;
}

void dvar_set_version_string(const game::dvar_t* dvar, const char* /*value*/) {
  const auto* result = get_version_string();
  utils::hook::invoke<void>(0x480E70, dvar, result);
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x46F570, get_build_number, HOOK_JUMP).install()->quick();
    utils::hook(0x60429A, dvar_set_version_string, HOOK_CALL)
        .install()
        ->quick();

    utils::hook::set<const char*>(0x446A48, "IW4x-SP: Console");
    utils::hook::set<const char*>(0x50C110, "IW4x-SP: Game");

    utils::hook::set<const char*>(0x579364, "IW4x-SP: " SHORTVERSION "> ");
  }
};
} // namespace branding

REGISTER_COMPONENT(branding::component)
