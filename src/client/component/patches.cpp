#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace patches {
namespace {
void sys_init_stub() {
  game::Cmd_SetAutoComplete("exec", "", "cfg");
  utils::hook::invoke<void>(0x4EEB20);
}

void cl_play_unskippable_cinematic_f() {
  // CL_PlayCinematic_f
  utils::hook::invoke<void>(0x4CC950);
  *game::cin_skippable = true;
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x6042A2, sys_init_stub, HOOK_CALL).install()->quick();

    patch_sp();
  }

private:
  static void patch_sp() {
    // Force external console
    utils::hook::nop(0x604071, 21);

    // Remove limit on IWD file loading
    utils::hook::set<std::uint8_t>(0x630FF3, 0xEB);

    // Ignore XUID match
    utils::hook::set<std::uint8_t>(0x65D1AF, 0xEB);

    // Remove this particular string from the log file
    utils::hook::nop(0x6030A6, 5);

    // Config related
    utils::hook::set<std::uint8_t>(0x4D3FD3, 0xEB);

    // Improper quit popup
    utils::hook::nop(0x4F5B3A, 2);

    // Remove fs_game check
    utils::hook::nop(0x612932, 2);

    // Build os path stuff
    utils::hook::set<std::uint8_t>(0x6300BF, 0xEB);

    // Show intro (or not)
    utils::hook::set<std::uint8_t>(0x6035BD, 0x0);

    // raw -> main
    utils::hook::set<std::uint32_t>(0x50A0B2, 0x723390);

    // Disable 'replay' output
    utils::hook::nop(0x65F14A, 5);

    // Disable heartbeat output
    utils::hook::nop(0x57EF87, 5);

    // Allow intro to be skipped
    utils::hook::set<void (*)()>(0x47529F, cl_play_unskippable_cinematic_f);

    // Enable commandline arguments
    utils::hook::set<std::uint8_t>(0x453B24, 0xEB);
  }
};
} // namespace patches

REGISTER_COMPONENT(patches::component)
