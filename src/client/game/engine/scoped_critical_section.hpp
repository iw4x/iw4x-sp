#pragma once

namespace game::engine {
class scoped_critical_section {
public:
  scoped_critical_section(CriticalSection s,
                          ScopedCriticalSectionType type) noexcept;
  ~scoped_critical_section() noexcept;

  void enter_crit_sect() noexcept;
  void leave_crit_sect() noexcept;
  bool try_enter_crit_sect() noexcept;

  [[nodiscard]] bool has_ownership() const noexcept;
  [[nodiscard]] bool is_scoped_release() const noexcept;

private:
  CriticalSection s_;
  bool has_ownership_;
  bool is_scoped_release_;
};
} // namespace game::engine
