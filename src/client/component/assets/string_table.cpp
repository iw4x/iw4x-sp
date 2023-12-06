#include <std_include.hpp>

#include "string_table.hpp"

#include <utils/flags.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

namespace assets {
namespace {
void dump_string_table(game::XAssetHeader* header) {
  const auto* string_table = header->stringTable;
  const auto filename = utils::string::va("raw/{0}", string_table->name);

  std::string csv;

  const auto rows = string_table->rowCount;
  const auto columns = string_table->columnCount;

  for (auto x = 0; x < rows; ++x) {
    for (auto y = 0; y < columns; ++y) {
      const char* cell = string_table->values[(x * columns) + y].string;
      csv += cell;

      if (y + 1 < columns) {
        csv += ",";
      }
    }

    if (x + 1 < rows) {
      csv += "\n";
    }
  }

  utils::io::write_file(filename, csv);
}

bool is_enabled() { IS_FLAG_ENABLED(dump_string_table); }
} // namespace

void process_string_table(game::XAssetHeader* header) {
  if (is_enabled()) {
    dump_string_table(header);
  }
}
} // namespace assets
