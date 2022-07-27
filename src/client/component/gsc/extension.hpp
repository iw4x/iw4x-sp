#pragma once

namespace gsc {
void add_function(const char* name, game::BuiltinFunction func,
                  bool type = false);
void add_method(const char* name, game::BuiltinMethod func, bool type = false);
} // namespace gsc
