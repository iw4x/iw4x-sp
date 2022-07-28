#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace player_movement {
namespace {
void __declspec(naked) pm_step_slide_move_stub() {
  __asm {
    push eax;
    mov eax, dvars::pm_bounce;
    cmp byte ptr [eax + 0x10], 1;
    pop eax;

    je bounce;

    cmp dword ptr [esp + 0x24], 0;
    jnz no_bounce;

   bounce:
    push 0x4E905B;
    ret;

   no_bounce:
    push 0x4E906F;
    ret;
  }
}

void pm_project_velocity_stub(const float* vel_in, const float* normal,
                              float* vel_out) {
  const auto length_squared_2d = vel_in[0] * vel_in[0] + vel_in[1] * vel_in[1];

  if (std::fabsf(normal[2]) < 0.001f || length_squared_2d == 0.0f) {
    vel_out[0] = vel_in[0];
    vel_out[1] = vel_in[1];
    vel_out[2] = vel_in[2];
    return;
  }

  auto new_z = vel_in[0] * normal[0] + vel_in[1] * normal[1];
  new_z = -new_z / normal[2];

  const auto length_scale =
      std::sqrtf((vel_in[2] * vel_in[2] + length_squared_2d) /
                 (new_z * new_z + length_squared_2d));

  if (dvars::pm_bouncingAllAngles->current.enabled ||
      (length_scale < 1.f || new_z < 0.f || vel_in[2] > 0.f)) {
    vel_out[0] = vel_in[0] * length_scale;
    vel_out[1] = vel_in[1] * length_scale;
    vel_out[2] = new_z * length_scale;
  }
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    dvars::pm_bounce = game::Dvar_RegisterBool("pm_bounce", false,
                                               game::DVAR_NONE, "CoD4 Bounces");
    dvars::pm_bouncingAllAngles =
        game::Dvar_RegisterBool("pm_bouncingAllAngles", false, game::DVAR_NONE,
                                "Enable bouncing from all angles");

    utils::hook(0x4E9054, pm_step_slide_move_stub, HOOK_JUMP)
        .install()
        ->quick(); // PM_StepSlideMove
    utils::hook(0x4E90BE, pm_project_velocity_stub, HOOK_CALL)
        .install()
        ->quick(); // PM_StepSlideMove
  }
};
} // namespace player_movement

REGISTER_COMPONENT(player_movement::component)
