#include "std_include.hpp"
#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include "launcher/launcher.hpp"

#include <utils/string.hpp>
#include <utils/flags.hpp>
#include <utils/nt.hpp>
#include <utils/io.hpp>

#include <gsl/gsl>

DECLSPEC_NORETURN void WINAPI exit_hook(const int code) {
  component_loader::pre_destroy();
  std::exit(code);
}

launcher::mode detect_mode_from_arguments() {
  if (utils::flags::has_flag("singleplayer")) {
    return launcher::mode::singleplayer;
  }

  return launcher::mode::none;
}

FARPROC load_binary(const launcher::mode mode) {
  loader loader;
  utils::nt::library self;

  loader.set_import_resolver(
      [self](const std::string& library, const std::string& function) -> void* {
        if (library == "steam_api.dll") {
          return self.get_proc<FARPROC>(function);
        }
        if (function == "ExitProcess") {
          return exit_hook;
        }

        return component_loader::load_import(library, function);
      });

  std::string binary;
  switch (mode) {
  case launcher::mode::singleplayer:
    binary = "iw4sp.exe";
    break;
  case launcher::mode::none:
    throw std::runtime_error("Invalid game mode!");
  }

  std::string data;
  if (!utils::io::read_file(binary, &data)) {
    throw std::runtime_error(
        utils::string::va("Failed to read game binary (%s)!\nPlease select the "
                          "correct path in the launcher settings.",
                          binary.data()));
  }

  return loader.load(self, data);
}

void enable_dpi_awareness() {
  const utils::nt::library user32{"user32.dll"};
  const auto set_dpi =
      user32 ? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
                   "SetProcessDpiAwarenessContext")
             : nullptr;
  if (set_dpi) {
    set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  }
}

void apply_environment() {
  char* buffer{};
  std::size_t size{};
  if (_dupenv_s(&buffer, &size, "XLABS_MW2_INSTALL") != 0 ||
      buffer == nullptr) {
    throw std::runtime_error("Please use the X Labs launcher to run the game!");
  }

  const auto _ = gsl::finally([&] { std::free(buffer); });

  std::string dir{buffer, size};
  SetCurrentDirectoryA(dir.data());
  SetDllDirectoryA(dir.data());
}

int main() {
  FARPROC entry_point;
  enable_dpi_awareness();

  std::srand(uint32_t(time(nullptr)));

  {
    auto premature_shutdown = true;
    const auto _ = gsl::finally([&premature_shutdown] {
      if (premature_shutdown) {
        component_loader::pre_destroy();
      }
    });

    try {
#ifdef CI
      apply_environment();
#endif

      if (!component_loader::post_start())
        return 0;

      auto mode = detect_mode_from_arguments();
      if (mode == launcher::mode::none) {
        const launcher launcher;
        mode = launcher.run();
        if (mode == launcher::mode::none)
          return 0;
      }

      game::environment::set_mode(mode);
      entry_point = load_binary(mode);
      if (!entry_point) {
        throw std::runtime_error("Unable to load binary into memory");
      }

      if (!component_loader::post_load())
        return 0;

      premature_shutdown = false;
    } catch (const std::exception& ex) {
      MessageBoxA(nullptr, ex.what(), "ERROR", MB_ICONERROR);
      return 1;
    }
  }

  return entry_point();
}
