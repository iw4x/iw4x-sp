#pragma once

#include <launcher/launcher.hpp>

namespace game {
namespace environment {
bool is_sp();

launcher::mode get_mode();
void set_mode(launcher::mode mode);
} // namespace environment

template <typename T> class symbol {
public:
  symbol(const size_t sp) : sp_(reinterpret_cast<T*>(sp)) {}

  [[nodiscard]] T* get() const { return sp_; }

  operator T*() const { return this->get(); }

  T* operator->() const { return this->get(); }

private:
  T* sp_;
};

// Functions that require asm stubs to be called
// or have been inlined

int FS_FOpenFileReadForThread(const char* filename, int* file, FsThread thread);

void IN_KeyDown(kbutton_t* b);
void IN_KeyUp(kbutton_t* b);

bool ScrPlace_IsFullScreenActive();
ScreenPlacement* ScrPlace_GetUnsafeFullPlacement();

bool Sys_TryEnterCriticalSection(CriticalSection critSect);

// Global definitions
constexpr auto CMD_MAX_NESTING = 8;

constexpr std::size_t MAX_LOCAL_CLIENTS = 1;
} // namespace game

#include "symbols.hpp"
