#include <std_include.hpp>

#include "component/filesystem.hpp"

#include "map_ents.hpp"

#include <utils/string.hpp>

namespace assets {
namespace {
std::string map_entities;

void load_map_entities(game::MapEnts* entry) {
  const auto file_name = utils::string::va("{0}.ents", entry->name);
  const filesystem::file ent_file(file_name, game::FS_THREAD_DATABASE);

  // Load ent file from raw if it exists
  if (ent_file.exists()) {
    map_entities = ent_file.get_buffer();
    entry->entityString = map_entities.data();
    entry->numEntityChars = static_cast<int>(map_entities.size()) + 1;
  }
}
} // namespace

void process_map_ents(game::XAssetHeader header) {
  auto* map_ents = header.mapEnts;
  load_map_entities(map_ents);
}
} // namespace assets
