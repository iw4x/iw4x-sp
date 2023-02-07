#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "localize_entry.hpp"
#include "map_ents.hpp"
#include "raw_file.hpp"

#include <utils/hook.hpp>

namespace assets {
namespace {
void load_asset(game::XAssetType type, game::XAssetHeader* header) {
  if (header) {
    switch (type) {
    case game::ASSET_TYPE_LOCALIZE_ENTRY:
      process_localize_entry(*header);
      break;
    case game::ASSET_TYPE_MAP_ENTS:
      process_map_ents(*header);
      break;
    case game::ASSET_TYPE_RAWFILE:
      process_raw_file(*header);
      break;
    default:
      break;
    }
  }
}

void __declspec(naked) db_add_x_asset_stub() {
  __asm {
    pushad;

    push [esp + 0x20 + 0x8];
    push [esp + 0x20 + 0x8];
    call load_asset;
    add esp, 0x8;

    popad;

    sub esp, 0x14;
    mov eax, dword ptr [esp + 0x1C];

    push 0x581EE7;
    ret;
  }
}
} // namespace

class asset final : public component_interface {
public:
  void post_load() override {
    // We may modify assets to some extents here or just dump them
    utils::hook(0x581EE0, db_add_x_asset_stub, HOOK_JUMP).install()->quick();
    utils::hook::nop(0x581EE5, 2);
  }
};
} // namespace assets

REGISTER_COMPONENT(assets::asset)
