#pragma once

#define CHUNK 16384u

namespace utils::compression {
namespace zlib {
std::string compress(const std::string& data);
std::string decompress(const std::string& data);
} // namespace zlib

namespace zip {
class archive {
public:
  void add(const std::string& filename, const std::string& data);
  bool write(const std::string& filename, const std::string& comment = {});

private:
  std::unordered_map<std::string, std::string> files_;
};
} // namespace zip
} // namespace utils::compression
