#include <std_include.hpp>

namespace dvars {
const game::dvar_t* r_noBorder = nullptr;

const game::dvar_t* pm_bounce = nullptr;
const game::dvar_t* pm_bouncingAllAngles = nullptr;
const game::dvar_t* pm_rocketJump = nullptr;
const game::dvar_t* pm_rocketJumpScale = nullptr;
const game::dvar_t* pm_playerCollision = nullptr;
const game::dvar_t* pm_elevators = nullptr;

const game::dvar_t* cg_drawVersion = nullptr;
const game::dvar_t* cg_drawVersionX = nullptr;
const game::dvar_t* cg_drawVersionY = nullptr;

const game::dvar_t* bug_name = nullptr;

const game::dvar_t* g_log = nullptr;

// Game dvars
const game::dvar_t** g_specialops =
    reinterpret_cast<const game::dvar_t**>(0x1B2E1E8);

const game::dvar_t** sv_mapname =
    reinterpret_cast<const game::dvar_t**>(0x1B2E1E4);

const game::dvar_t** version =
    reinterpret_cast<const game::dvar_t**>(0x145D690);
} // namespace dvars
