#include "string.hpp"
#include <sstream>
#include <algorithm>

#include "nt.hpp"

namespace utils::string {
std::vector<std::string> split(const std::string& s, const char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;

  while (std::getline(ss, item, delim)) {
    elems.push_back(item); // elems.push_back(std::move(item)); // if C++11
                           // (based on comment from @mchiasson)
  }

  return elems;
}

std::string to_lower(const std::string& text) {
  std::string result;
  std::ranges::transform(text, std::back_inserter(result),
                         [](const unsigned char input) {
                           return static_cast<char>(std::tolower(input));
                         });

  return result;
}

std::string to_upper(const std::string& text) {
  std::string result;
  std::ranges::transform(text, std::back_inserter(result),
                         [](const unsigned char input) {
                           return static_cast<char>(std::toupper(input));
                         });

  return result;
}

bool starts_with(const std::string& text, const std::string& substring) {
  return text.find(substring) == 0;
}

bool ends_with(const std::string& text, const std::string& substring) {
  if (substring.size() > text.size())
    return false;
  return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
}

bool compare(const std::string& lhs, const std::string& rhs) {
  return std::ranges::equal(lhs, rhs,
                            [](const unsigned char a, const unsigned char b) {
                              return std::tolower(a) == std::tolower(b);
                            });
}

std::string dump_hex(const std::string& data, const std::string& separator) {
  char buf[64]{};
  std::string result;

  for (unsigned int i = 0; i < data.size(); ++i) {
    if (i > 0) {
      result.append(separator);
    }

    sprintf_s(buf, "%02X", data[i] & 0xFF);
    result.append(buf);
  }

  return result;
}

std::string get_clipboard_data() {
  if (OpenClipboard(nullptr)) {
    std::string data;

    auto* const clipboard_data = GetClipboardData(1u);
    if (clipboard_data) {
      auto* const cliptext = static_cast<char*>(GlobalLock(clipboard_data));
      if (cliptext) {
        data.append(cliptext);
        GlobalUnlock(clipboard_data);
      }
    }
    CloseClipboard();

    return data;
  }
  return {};
}

std::string convert(const std::wstring& wstr) {
  std::string result;
  result.reserve(wstr.size());

  for (const auto& chr : wstr) {
    result.push_back(static_cast<char>(chr));
  }

  return result;
}

std::wstring convert(const std::string& str) {
  std::wstring result;
  result.reserve(str.size());

  for (const auto& chr : str) {
    result.push_back(static_cast<wchar_t>(chr));
  }

  return result;
}

std::string replace(std::string str, const std::string& from,
                    const std::string& to) {
  if (from.empty()) {
    return str;
  }

  std::size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }

  return str;
}
} // namespace utils::string
