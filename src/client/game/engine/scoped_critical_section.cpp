#include <std_include.hpp>

#include "scoped_critical_section.hpp"

namespace game::engine {
scoped_critical_section::scoped_critical_section(
    const CriticalSection s, const ScopedCriticalSectionType type) noexcept
    : s_(s), is_scoped_release_(false) {
  if (type == SCOPED_CRITSECT_NORMAL) {
    Sys_EnterCriticalSection(this->s_);
    this->has_ownership_ = true;
  } else {
    if (type == SCOPED_CRITSECT_TRY) {
      this->has_ownership_ = Sys_TryEnterCriticalSection(this->s_);
    } else {
      if (type == SCOPED_CRITSECT_RELEASE) {
        Sys_LeaveCriticalSection(this->s_);
        this->is_scoped_release_ = true;
      }

      this->has_ownership_ = false;
    }
  }
}

scoped_critical_section::~scoped_critical_section() noexcept {
  if (!this->has_ownership_ || this->is_scoped_release_) {
    if (!this->has_ownership_ && this->is_scoped_release_) {
      Sys_EnterCriticalSection(this->s_);
    }
  } else {
    Sys_LeaveCriticalSection(this->s_);
  }
}

void scoped_critical_section::enter_crit_sect() noexcept {
  assert(!this->has_ownership_);

  this->has_ownership_ = true;
  Sys_EnterCriticalSection(this->s_);
}

void scoped_critical_section::leave_crit_sect() noexcept {
  assert(this->has_ownership_);

  this->has_ownership_ = false;
  Sys_LeaveCriticalSection(this->s_);
}

bool scoped_critical_section::try_enter_crit_sect() noexcept {
  assert(!this->has_ownership_);

  const auto result = Sys_TryEnterCriticalSection(this->s_);
  this->has_ownership_ = result;
  return result;
}

bool scoped_critical_section::has_ownership() const noexcept {
  return this->has_ownership_;
}

bool scoped_critical_section::is_scoped_release() const noexcept {
  return this->is_scoped_release_;
}
} // namespace game::engine
