#pragma once

namespace game::engine {
class fast_critical_section_scope_read {
public:
  fast_critical_section_scope_read(FastCriticalSection* cs) noexcept;
  ~fast_critical_section_scope_read() noexcept;

private:
  FastCriticalSection* cs_;
};

class fast_critical_section_scope_write {
public:
  fast_critical_section_scope_write(FastCriticalSection* cs) noexcept;
  ~fast_critical_section_scope_write() noexcept;

private:
  FastCriticalSection* cs_;
};
} // namespace game::engine
