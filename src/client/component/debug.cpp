#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace debug {
namespace {
void com_assert_f() { assert(("a", false)); }
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook::set<void (*)()>(0x604203, com_assert_f);
  }
};
} // namespace debug

REGISTER_COMPONENT(debug::component)
