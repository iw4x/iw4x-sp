#include <std_include.hpp>

namespace game {
namespace environment {
launcher::mode mode = launcher::mode::none;

bool is_sp() { return get_mode() == launcher::mode::singleplayer; }

launcher::mode get_mode() {
  if (mode == launcher::mode::none) {
    throw std::runtime_error(
        "Launcher mode not valid. Something must be wrong.");
  }

  return mode;
}

void set_mode(const launcher::mode _mode) { mode = _mode; }
} // namespace environment

int FS_FOpenFileReadForThread(const char* filename, int* file,
                              FsThread thread) {
  const static DWORD func = 0x630380;
  int answer{};

  __asm {
    pushad;

    mov eax, file;
    push thread;
    push filename;
    call func;
    add esp, 0x8;
    mov answer, eax;

    popad;
  }

  return answer;
}

void IN_KeyDown(kbutton_t* b) {
  const static DWORD func = 0x57A350;

  __asm {
    pushad;

    mov esi, b;
    call func;

    popad;
  }
}

void IN_KeyUp(kbutton_t* b) {
  const static DWORD func = 0x57A3F0;

  __asm {
    pushad;

    mov esi, b;
    call func;

    popad;
  }
}

bool ScrPlace_IsFullScreenActive() {
  return *activeScreenPlacementMode == SCRMODE_FULL;
}

ScreenPlacement* ScrPlace_GetUnsafeFullPlacement() {
  return scrPlaceFullUnsafe;
}

bool Sys_TryEnterCriticalSection(CriticalSection critSect) {
  assert(static_cast<unsigned>(critSect) <
         static_cast<unsigned>(CRITSECT_COUNT));

  return TryEnterCriticalSection(&s_criticalSection[critSect]) != FALSE;
}
} // namespace game
