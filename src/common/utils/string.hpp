#pragma once
#include "memory.hpp"
#include <iterator>
#include <string>
#include <string_view>
#include <format>

namespace utils::string {

template <typename Arg> // This should display a nice "null" instead of a number
static void sanitize_format_args(Arg& arg) {
  if constexpr (std::is_same_v<Arg, char*> ||
                std::is_same_v<Arg, const char*>) {
    if (arg == nullptr) {
      arg = const_cast<char*>("nullptr");
    }
  }
}

template <typename... Args>
static const char* va(std::string_view fmt, Args&&... args) {
  static thread_local std::string va_buffer;

  (sanitize_format_args(args), ...);
  std::vformat(fmt, std::make_format_args(args...)).swap(va_buffer);
  return va_buffer.data();
}

std::vector<std::string> split(const std::string& s, char delim);

std::string to_lower(const std::string& text);
std::string to_upper(const std::string& text);

bool starts_with(const std::string& text, const std::string& substring);
bool ends_with(const std::string& text, const std::string& substring);
bool compare(const std::string& lhs, const std::string& rhs);

std::string dump_hex(const std::string& data,
                     const std::string& separator = " ");

std::string get_clipboard_data();

std::string convert(const std::wstring& wstr);
std::wstring convert(const std::string& str);

std::string replace(std::string str, const std::string& from,
                    const std::string& to);
} // namespace utils::string
