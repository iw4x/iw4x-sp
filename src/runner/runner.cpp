#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>

#include "debugger.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
  const auto* const command = "-proc ";
  const char* parent_proc = std::strstr(GetCommandLineA(), command);

  if (parent_proc) {
    const auto pid = DWORD(atoi(parent_proc + std::strlen(command)));
    auto* const process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (process_handle) {
      WaitForSingleObject(process_handle, INFINITE);
      CloseHandle(process_handle);
      return 0;
    }
  }

  return 1;
}
