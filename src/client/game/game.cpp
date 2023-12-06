#include <std_include.hpp>

namespace game {
int FS_FOpenFileReadForThread(const char* filename, int* file,
                              FsThread thread) {
  const static DWORD FS_FOpenFileReadForThread_t = 0x630380;
  int result{};

  __asm {
    pushad;

    mov eax, file;
    push thread;
    push filename;
    call FS_FOpenFileReadForThread_t;
    add esp, 0x8;
    mov result, eax;

    popad;
  }

  return result;
}

void IN_KeyDown(kbutton_t* b) {
  const static DWORD IN_KeyDown_t = 0x57A350;

  __asm {
    pushad;

    mov esi, b;
    call IN_KeyDown_t;

    popad;
  }
}

void IN_KeyUp(kbutton_t* b) {
  const static DWORD IN_KeyUp_t = 0x57A3F0;

  __asm {
    pushad;

    mov esi, b;
    call IN_KeyUp_t;

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

void Sys_LockRead(FastCriticalSection* critSect) {
  InterlockedIncrement(&critSect->readCount);
  while (critSect->writeCount) {
    Sys_Sleep(0);
  }
}

void Sys_UnlockRead(FastCriticalSection* critSect) {
  assert(critSect->readCount > 0);
  InterlockedDecrement(&critSect->readCount);
}

void Sys_UnlockWrite(FastCriticalSection* critSect) {
  assert(critSect->writeCount > 0);
  InterlockedDecrement(&critSect->writeCount);
  Sys_TempPriorityEnd(&critSect->tempPriority);
}

void Sys_SnapVector(float* v) {
  v[0] = std::floorf(v[0] + 0.5f);
  v[1] = std::floorf(v[1] + 0.5f);
  v[2] = std::floorf(v[2] + 0.5f);
}

int PC_Int_Parse(int handle, int* i) {
  const static DWORD PC_Int_Parse_t = 0x62DF10;
  int result{};

  __asm {
     pushad;

     mov eax, handle;
     mov esi, i;
     call PC_Int_Parse_t;
     mov result, eax;

     popad;
  }

  return result;
}

int PC_Float_Parse(int handle, float* f) {
  const static DWORD PC_Float_Parse_t = 0x62DE40;
  int result{};

  __asm {
    pushad;

    mov eax, handle;
    mov esi, f;
    call PC_Float_Parse_t;
    mov result, eax;

    popad;
  }

  return result;
}

void Menu_FreeItemMemory(itemDef_s* item) {
  const static DWORD Menu_FreeItemMemory_t = 0x62B7E0;

  __asm {
    pushad;

    mov edi, item;
    call Menu_FreeItemMemory_t;

    popad;
  }
}

} // namespace game
