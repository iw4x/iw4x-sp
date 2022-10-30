#include <std_include.hpp>

namespace game {
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
  assert(static_cast<std::uint32_t>(critSect) <
         static_cast<std::uint32_t>(CRITSECT_COUNT));

  return TryEnterCriticalSection(&s_criticalSection[critSect]) != FALSE;
}
} // namespace game
