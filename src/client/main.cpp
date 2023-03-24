#include "std_include.hpp"
#include <utils/nt.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include "loader/binary_loader.hpp"
#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include <gsl/gsl>

#include <version.hpp>
#include <DbgHelp.h>

LONG WINAPI exception_handler(PEXCEPTION_POINTERS exception_info) {
  if (exception_info->ExceptionRecord->ExceptionCode == 0x406D1388) {
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  if (exception_info->ExceptionRecord->ExceptionCode < 0x80000000 ||
      exception_info->ExceptionRecord->ExceptionCode == 0xE06D7363) {
    return EXCEPTION_CONTINUE_SEARCH;
  }

  MINIDUMP_EXCEPTION_INFORMATION exception_information = {
      GetCurrentThreadId(), exception_info, FALSE};
  const auto type = MiniDumpIgnoreInaccessibleMemory //
                    | MiniDumpWithHandleData         //
                    | MiniDumpScanMemory             //
                    | MiniDumpWithProcessThreadData  //
                    | MiniDumpWithFullMemoryInfo     //
                    | MiniDumpWithThreadInfo;

  CreateDirectoryA("minidumps", nullptr);
  const auto file_name =
      utils::string::va("minidumps\\iw4x-sp_{0}.dmp", SHORTVERSION);
  constexpr auto file_share = FILE_SHARE_READ | FILE_SHARE_WRITE;
  const auto file_handle =
      CreateFileA(file_name, GENERIC_WRITE | GENERIC_READ, file_share, nullptr,
                  CREATE_ALWAYS, NULL, nullptr);

  if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                         file_handle, static_cast<MINIDUMP_TYPE>(type),
                         &exception_information, nullptr, nullptr)) {
    char buf[4096]{};
    sprintf_s(buf, "An exception 0x%08X occurred at location 0x%p\n",
              exception_info->ExceptionRecord->ExceptionCode,
              exception_info->ExceptionRecord->ExceptionAddress);
    MessageBoxA(nullptr, buf, "Fatal Error", MB_ICONERROR);
  }

  CloseHandle(file_handle);
  TerminateProcess(GetCurrentProcess(),
                   exception_info->ExceptionRecord->ExceptionCode);

  return EXCEPTION_CONTINUE_SEARCH;
}

DECLSPEC_NORETURN void WINAPI exit_hook(const int code) {
  component_loader::pre_destroy();
  std::exit(code);
}

FARPROC load_binary() {
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

  const auto buffer = binary_loader::load();
  return loader.load(self, buffer);
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
    return;
  }

  const auto _ = gsl::finally([&] { std::free(buffer); });

  SetCurrentDirectoryA(buffer);
  SetDllDirectoryA(buffer);
}

int main() {
  AddVectoredExceptionHandler(0, exception_handler);
  SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

  FARPROC entry_point;
  enable_dpi_awareness();

  std::srand(std::uint32_t(time(nullptr)) ^
             ~(GetTickCount() * GetCurrentProcessId()));

  {
    auto premature_shutdown = true;
    const auto _0 = gsl::finally([&premature_shutdown] {
      if (premature_shutdown) {
        component_loader::pre_destroy();
      }
    });

    try {
      apply_environment();

      if (!component_loader::post_start())
        return 0;

      entry_point = load_binary();
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

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int) { return main(); }
