#include "csv.hpp"
#include "io.hpp"
#include "string.hpp"

namespace utils {
csv::csv(const std::string& file, const bool is_file, const bool allow_comments)
    : valid_(false) {
  this->parse(file, is_file, allow_comments);
}

std::size_t csv::get_rows() const noexcept { return this->data_map_.size(); }

std::size_t csv::get_columns() const {
  std::size_t count = 0;

  for (std::size_t i = 0; i < this->get_rows(); ++i) {
    count = std::max(this->get_columns(i), count);
  }

  return count;
}

std::size_t csv::get_columns(const std::size_t row) const {
  if (this->data_map_.size() > row) {
    return this->data_map_[row].size();
  }

  return 0;
}

std::string csv::get_element_at(const std::size_t row,
                                const std::size_t column) const {
  if (this->data_map_.size() > row) {
    const auto& data = this->data_map_[row];
    if (data.size() > column) {
      return data[column];
    }
  }

  return {};
}

bool csv::is_valid() const noexcept { return this->valid_; }

void csv::parse(const std::string& file, const bool is_file,
                const bool allow_comments) {
  std::string buffer;

  if (is_file) {
    if (io::read_file(file, &buffer) && !buffer.empty()) {
      this->valid_ = true;
    }
  } else {
    buffer = file;
  }

  if (!buffer.empty()) {
    const auto rows = string::split(buffer, '\n');

    for (auto& row : rows) {
      this->parse_row(row, allow_comments);
    }
  }
}

void csv::parse_row(const std::string& row, const bool allow_comments) {
  bool is_string = false;
  std::string element;
  std::vector<std::string> data;
  char temp_char = '\0';

  for (std::size_t i = 0; i < row.size(); ++i) {
    if (row[i] == ',' && !is_string) // Flush entry
    {
      data.push_back(element);
      element.clear();
      continue;
    }

    if (row[i] == '"') // Start / Terminate string
    {
      is_string = !is_string;
      continue;
    }

    if (i < (row.size() - 1) && row[i] == '\\' && row[i + 1] == '"' &&
        is_string) // Handle quotes in strings as \"
    {
      temp_char = '"';
      ++i;
    } else if (!is_string && (row[i] == '\n' || row[i] == '\x0D' ||
                              row[i] == '\x0A' || row[i] == '\t')) {
      continue;
    } else if (!is_string &&
               (row[i] == '#' ||
                (row[i] == '/' && (i + 1) < row.size() && row[i + 1] == '/')) &&
               allow_comments) {
      // Skip comments. I know CSVs usually don't have comments, but in this
      // case it's useful
      return;
    } else {
      temp_char = row[i];
    }

    element.push_back(temp_char);
  }

  // Push last element
  data.push_back(element);

  if (data.empty() || (data.size() == 1 && data[0].empty())) // Skip empty rows
  {
    return;
  }

  this->data_map_.push_back(data);
}
} // namespace utils
