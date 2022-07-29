#pragma once

namespace filesystem {
std::vector<std::string> vectored_file_list(const std::string& path,
                                            const std::string& extension);

class file {
public:
  file(std::string name, game::FsThread thread);

  [[nodiscard]] bool exists() const;
  [[nodiscard]] const std::string& get_buffer() const;
  [[nodiscard]] const std::string& get_name() const;

private:
  bool valid_ = false;
  std::string name_;
  std::string buffer_;
};
} // namespace filesystem
