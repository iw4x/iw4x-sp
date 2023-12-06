#include <std_include.hpp>
#include <game/game.hpp>

#include "fast_critical_section.hpp"

namespace game::engine {
fast_critical_section_scope_read::fast_critical_section_scope_read(
    FastCriticalSection* cs) noexcept
    : cs_(cs) {
  if (this->cs_) {
    Sys_LockRead(this->cs_);
  }
}

fast_critical_section_scope_read::~fast_critical_section_scope_read() noexcept {
  if (this->cs_) {
    Sys_UnlockRead(this->cs_);
  }
}

fast_critical_section_scope_write::fast_critical_section_scope_write(
    FastCriticalSection* cs) noexcept
    : cs_(cs) {
  if (this->cs_) {
    Sys_LockWrite(this->cs_);
  }
}

fast_critical_section_scope_write::
    ~fast_critical_section_scope_write() noexcept {
  if (this->cs_) {
    Sys_UnlockWrite(this->cs_);
  }
}
} // namespace game::engine
