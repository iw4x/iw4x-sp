#pragma once

namespace dvars {
extern const game::dvar_t* r_noBorder;

extern const game::dvar_t* pm_bounce;
extern const game::dvar_t* pm_bouncingAllAngles;
extern const game::dvar_t* pm_rocketJump;
extern const game::dvar_t* pm_rocketJumpScale;
extern const game::dvar_t* pm_playerCollision;
extern const game::dvar_t* pm_elevators;
extern const game::dvar_t* pm_disableLandingSlowdown;
extern const game::dvar_t* pm_bunnyHop;
extern const game::dvar_t* pm_snapVector;

extern const game::dvar_t* cg_drawVersion;
extern const game::dvar_t* cg_drawVersionX;
extern const game::dvar_t* cg_drawVersionY;

extern const game::dvar_t* bug_name;

extern const game::dvar_t* g_log;

// Game dvars
extern const game::dvar_t** g_specialops;

extern const game::dvar_t** sv_mapname;

extern const game::dvar_t** version;

extern const game::dvar_t** com_developer;
extern const game::dvar_t** com_developer_script;
} // namespace dvars
