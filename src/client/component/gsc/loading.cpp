#include <std_include.hpp>
#include "../../loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "../filesystem.hpp"

namespace gsc {
namespace {
std::vector<int> main_handles;
std::vector<int> init_handles;

void g_scr_load_scripts_stub() {
  // Clear handles (from previous GSC loading session)
  main_handles.clear();
  init_handles.clear();

  const auto file_list = filesystem::vectored_file_list("scripts/", "gsc");

  for (const auto& file : file_list) {
    std::string script = "scripts/" + file;

    if (script.ends_with(".gsc")) {
      // Scr_LoadScriptInternal will add the '.gsc' suffix
      script = script.substr(0, script.size() - 4);
    }

    game::Com_Printf(game::CON_CHANNEL_SERVER, "Loading script %s.gsc...\n",
                     script.data());

    if (!game::Scr_LoadScript(script.data())) {
      game::Com_Printf(game::CON_CHANNEL_SERVER,
                       "Script %s encountered an error while loading\n",
                       script.data());
      game::Com_Error(game::ERR_DROP, "Could not find script '%s'",
                      script.data());
      return;
    }

    game::Com_Printf(game::CON_CHANNEL_SERVER,
                     "Script %s.gsc loaded successfully\n", script.data());

    const auto main_handle = game::Scr_GetFunctionHandle(script.data(), "main");
    if (main_handle) {
      main_handles.push_back(main_handle);
    }

    const auto init_handle = game::Scr_GetFunctionHandle(script.data(), "init");
    if (init_handle) {
      init_handles.push_back(init_handle);
    }
    // Allow scripts with no handles
  }
}

void scr_load_level_stub() {
  for (const auto& handle : main_handles) {
    const auto id = game::Scr_ExecThread(handle, 0);
    game::Scr_FreeThread(static_cast<std::uint16_t>(id));
  }

  utils::hook::invoke<void>(0x470860); // Scr_LoadLevel

  for (const auto& handle : init_handles) {
    const auto id = game::Scr_ExecThread(handle, 0);
    game::Scr_FreeThread(static_cast<std::uint16_t>(id));
  }
}
} // namespace

class loading final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x4054C2, g_scr_load_scripts_stub, HOOK_JUMP)
        .install()
        ->quick();

    utils::hook(0x4C1E34, scr_load_level_stub, HOOK_CALL).install()->quick();
  }
};
} // namespace gsc

REGISTER_COMPONENT(gsc::loading)
