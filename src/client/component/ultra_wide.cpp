#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace ultra_wide {
const game::dvar_t* r_customAspectRatio;

const game::dvar_t* dvar_register_aspect_ratio(const char* dvar_name,
                                               const char** /*value_list*/,
                                               int default_index,
                                               unsigned __int16 flags,
                                               const char* description) {
  static const char* values[] = {"auto",      "standard", "wide 16:10",
                                 "wide 16:9", "custom",   nullptr};

  // register enumeration dvar
  return game::Dvar_RegisterEnum(dvar_name, values, default_index, flags,
                                 description);
}

void set_aspect_ratio() {
  *reinterpret_cast<float*>(0x1C91A78) = r_customAspectRatio->current.value;
}

__declspec(naked) void set_aspect_ratio_stub() {
  __asm {
        mov eax, [eax + 0x10];
        cmp eax, 4;

        mov dword ptr ds:0x1C91A68, edx;
        mov dword ptr ds:0x1C91A6C, esi;
        mov dword ptr ds:0x1C91A74, ecx;

        ja default_case;
        je custom_ratio;

        push 0x50AE6C;
        ret;

    default_case:
        push 0x50AF6C;
        ret;

    custom_ratio:
        pushad;
        call set_aspect_ratio;
        popad;

        mov eax, 1; // set widescreen to 1

        push 0x50AF05;
        ret;
  }
}

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x51E80B, &dvar_register_aspect_ratio, HOOK_CALL)
        .install()
        ->quick();

    utils::hook(0x50AE4E, &set_aspect_ratio_stub, HOOK_JUMP).install()->quick();
    utils::hook::nop(0x50AE4E + 5, 1);

    r_customAspectRatio = game::Dvar_RegisterFloat(
        "r_customAspectRatio", 16.0f / 9.0f, 4.0f / 3.0f, 63.0f / 9.0f,
        game::DVAR_ARCHIVE, "Screen aspect ratio");
  }
};
} // namespace ultra_wide

REGISTER_COMPONENT(ultra_wide::component)
