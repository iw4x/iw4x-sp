#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "dvar.hpp"

namespace console {
namespace {
bool is_command;

void con_toggle_console() {
  game::Field_Clear(game::g_consoleField);

  if (game::conDrawInputGlob->matchIndex >= 0 &&
      game::conDrawInputGlob->autoCompleteChoice[0] != '\0') {
    game::conDrawInputGlob->matchIndex = -1;
    game::conDrawInputGlob->autoCompleteChoice[0] = '\0';
  }

  game::g_consoleField->widthInPixels = *game::g_console_field_width;
  game::g_consoleField->charHeight = *game::g_console_char_height;
  game::g_consoleField->fixedSize = 1;
  game::con->outputVisible = false;

  // clientUIActives[0].keyCatchers
  utils::hook::set<std::uint32_t>(0x929140,
                                  *reinterpret_cast<std::uint32_t*>(0x929140) ^
                                      game::KEYCATCH_CONSOLE);
}

bool con_is_dvar_command_stub(const char* cmd) {
  is_command = game::Con_IsDvarCommand(cmd);
  return is_command;
}

void cmd_for_each_stub(void (*callback)(const char* str)) {
  if (!is_command) {
    utils::hook::invoke<void>(0x4B7000, callback);
  }
}
} // namespace

class component final : public component_interface {
public:
  static_assert(sizeof(game::field_t) == 0x118);
  static_assert(sizeof(game::ConDrawInputGlob) == 0x64);

  void post_start() override {
    // Prevents console from opening
    dvar::override::register_bool("monkeytoy", false, game::DVAR_NONE);
  }

  void post_load() override {
    utils::hook(0x44317E, con_toggle_console, HOOK_CALL)
        .install()
        ->quick(); // CL_KeyEvent
    utils::hook(0x442E8E, con_toggle_console, HOOK_JUMP)
        .install()
        ->quick(); // CL_KeyEvent

    // Con_DrawInput
    utils::hook(0x57946D, con_is_dvar_command_stub, HOOK_CALL)
        .install()
        ->quick();
    utils::hook(0x57951C, cmd_for_each_stub, HOOK_CALL).install()->quick();
  }
};
} // namespace console

REGISTER_COMPONENT(console::component)
