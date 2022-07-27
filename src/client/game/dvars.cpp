#include <std_include.hpp>

namespace dvars {
const game::dvar_t* r_noBorder = nullptr;

const game::dvar_t* pm_bounce = nullptr;
const game::dvar_t* pm_bouncingAllAngles = nullptr;

// Game dvars
const game::dvar_t** g_specialops =
    reinterpret_cast<const game::dvar_t**>(0x1B2E1E8);
} // namespace dvars
