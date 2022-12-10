#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/memory.hpp>

namespace asset_restrict {
namespace {
game::XAssetEntry entry_pool[789312];

game::XAssetHeader reallocate_asset_pool(game::XAssetType type,
                                         const int size) {
  const auto entry_size =
      reinterpret_cast<int (*)()>(game::DB_GetXAssetSizeHandlers[type])();
  const game::XAssetHeader pool_entry = {
      .data = utils::memory::allocate(entry_size * size)};
  game::DB_XAssetPool[type] = pool_entry.data;
  game::g_poolSize[type] = size;

  return pool_entry;
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    patch_entry_pool_sp();
    reallocate_asset_pool(game::ASSET_TYPE_GAMEWORLD_MP, 1);
  }

  static void patch_entry_pool_sp() {
    // Apply new size
    utils::hook::set<std::uint32_t>(0x581740, sizeof(entry_pool) /
                                                  sizeof(game::XAssetEntry));
    utils::hook::set<game::XAssetEntry*>(0x581721, entry_pool + 1);
    utils::hook::set<game::XAssetEntry*>(0x581732, entry_pool + 1);

    utils::hook::signature signature(0x411000, 0x200000);

    signature.add({"\x60\xB3\xB2\x00", "xxxx", [](char* address) {
                     utils::hook::set<game::XAssetEntry*>(address, entry_pool);
                   }});

    signature.process();
  }
};
} // namespace asset_restrict

REGISTER_COMPONENT(asset_restrict::component)
