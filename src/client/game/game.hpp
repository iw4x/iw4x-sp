#pragma once

namespace game {
template <typename T> struct base_symbol {
  base_symbol(const std::size_t address) : address_(address) {}

  [[nodiscard]] T* get() const { return reinterpret_cast<T*>(this->address_); }

  operator T*() const { return this->get(); }

  T* operator->() const { return this->get(); }

private:
  std::size_t address_{};
};

template <typename T> struct symbol : base_symbol<T> {
  using base_symbol<T>::base_symbol;
};

template <typename T, typename... Args>
struct symbol<T(Args...)> : base_symbol<T(Args...)> {
  using func_type = T(Args...);
  using base_symbol<func_type>::base_symbol;
};

// Functions that require asm stubs to be called
// or have been inlined

int FS_FOpenFileReadForThread(const char* filename, int* file, FsThread thread);

void IN_KeyDown(kbutton_t* b);
void IN_KeyUp(kbutton_t* b);

bool ScrPlace_IsFullScreenActive();
ScreenPlacement* ScrPlace_GetUnsafeFullPlacement();

bool Sys_TryEnterCriticalSection(CriticalSection critSect);

int PC_Int_Parse(int handle, int* i);
int PC_Float_Parse(int handle, float* f);

// Global definitions
constexpr auto CMD_MAX_NESTING = 8;

constexpr std::size_t MAX_LOCAL_CLIENTS = 1;

constexpr auto MAX_QPATH = 64;
} // namespace game

#include "symbols.hpp"
