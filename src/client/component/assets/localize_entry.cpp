#include <std_include.hpp>

#include "localize_entry.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/flags.hpp>

namespace assets {
namespace {
bool is_enabled() { IS_FLAG_ENABLED(dump_localize_entry); }
} // namespace

void process_localize_entry(game::XAssetHeader header) {
  if (!is_enabled()) {
    return;
  }

  auto* localize = header.localize;
  const auto filename =
      utils::string::va("raw/localizedstrings/{0}", localize->name);

  // Simple format, should be fine for now
  utils::io::write_file(filename, localize->value);
}
} // namespace assets
