#pragma once

namespace dvar {
namespace override {
void register_bool(const std::string& name, bool value, std::uint16_t flags);
void register_float(const std::string& name, float value, float min, float max,
                    std::uint16_t flags);
void register_int(const std::string& name, int value, int min, int max,
                  std::uint16_t flags);
void register_string(const std::string& name, const std::string& value,
                     std::uint16_t flags);
} // namespace override
} // namespace dvar
