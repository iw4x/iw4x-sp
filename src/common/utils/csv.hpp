#pragma once
#include <string>
#include <vector>

namespace utils {
class csv {
public:
  csv(const std::string& file, bool is_file = true, bool allow_comments = true);

  [[nodiscard]] std::size_t get_rows() const noexcept;
  [[nodiscard]] std::size_t get_columns() const;
  [[nodiscard]] std::size_t get_columns(std::size_t row) const;

  [[nodiscard]] std::string get_element_at(std::size_t row,
                                           std::size_t column) const;

  [[nodiscard]] bool is_valid() const noexcept;

private:
  bool valid_;
  std::vector<std::vector<std::string>> data_map_;

  void parse(const std::string& file, bool is_file = true,
             bool allow_comments = true);
  void parse_row(const std::string& row, bool allow_comments = true);
};
} // namespace utils
