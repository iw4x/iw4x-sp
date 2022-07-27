#pragma once

namespace dvar {
namespace override {
void dvar_register_bool(const std::string& name, bool value,
                        unsigned __int16 flags);
void dvar_register_float(const std::string& name, float value, float min,
                         float max, unsigned __int16 flags);
void dvar_register_int(const std::string& name, int value, int min, int max,
                       unsigned __int16 flags);
void dvar_register_string(const std::string& name, const std::string& value,
                          unsigned __int16 flags);
} // namespace override
} // namespace dvar
