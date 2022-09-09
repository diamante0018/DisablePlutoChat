#include <std_include.hpp>

#include "../loader/component_loader.hpp"
#include <utils/hook.hpp>

#include "command.hpp"

namespace movement
{
  game::dvar_t* player_spectateSpeedScale;
  game::dvar_t* cg_ufoScaler;
  game::dvar_t* cg_noclipScaler;
  game::dvar_t* bg_bouncesAllAngles;
  game::dvar_t* bg_elevators;

  float pm_move_scale(game::playerState_s* ps, float forward_move,
                      float right_move, float up_move)
  {
    assert(ps != nullptr);

    auto max = (std::fabsf(forward_move) < std::fabsf(right_move))
                   ? std::fabsf(right_move)
                   : std::fabsf(forward_move);

    if (std::fabsf(up_move) > max)
    {
      max = std::fabsf(up_move);
    }

    if (max == 0.0f)
    {
      return 0.0f;
    }

    const auto total = std::sqrtf(forward_move * forward_move +
                                  right_move * right_move + up_move * up_move);
    auto scale = (static_cast<float>(ps->speed) * max) / (127.0f * total);

    if (ps->pm_flags & game::PMF_WALKING)
    {
      scale *= 0.4f;
    }

    if (ps->pm_type == game::PM_NOCLIP)
    {
      return scale * cg_noclipScaler->current.value;
    }

    if (ps->pm_type == game::PM_UFO)
    {
      return scale * cg_ufoScaler->current.value;
    }

    if (ps->pm_type == game::PM_SPECTATOR)
    {
      return scale * player_spectateSpeedScale->current.value;
    }

    return scale;
  }

  __declspec(naked) void pm_move_scale_stub()
  {
    __asm
    {
      pushad

      push [esp + 0xC + 0x20]
      push [esp + 0xC + 0x20]
      push [esp + 0xC + 0x20]
      push esi
      call pm_move_scale
      add esp, 0x10

      popad
      ret
    }
  }

  void pm_project_velocity_stub(const float* vel_in, const float* normal,
                                float* vel_out)
  {
    const auto length_squared_2d =
        vel_in[0] * vel_in[0] + vel_in[1] * vel_in[1];

    if (std::fabsf(normal[2]) < 0.001f || length_squared_2d == 0.0f)
    {
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

    if (bg_bouncesAllAngles->current.enabled ||
        (length_scale < 1.f || new_z < 0.f || vel_in[2] > 0.f))
    {
      vel_out[0] = vel_in[0] * length_scale;
      vel_out[1] = vel_in[1] * length_scale;
      vel_out[2] = new_z * length_scale;
    }
  }

  void pm_trace_stub(game::pmove_t* pm, game::trace_t* results,
                     const float* start, const float* end,
                     const game::Bounds* bounds, int pass_entity_num,
                     int content_mask)
  {
    game::PM_trace(
        pm, results, start, end, bounds, pass_entity_num, content_mask);

    if (bg_elevators->current.enabled)
    {
      results->allsolid = false;
    }
  }

  void pm_player_trace_stub(game::pmove_t* pm, game::trace_t* results,
                            const float* start, const float* end,
                            const game::Bounds* bounds, int pass_entity_num,
                            int content_mask)
  {
    game::PM_playerTrace(
        pm, results, start, end, bounds, pass_entity_num, content_mask);

    if (bg_elevators->current.enabled)
    {
      results->startsolid = false;
    }
  }

  void cmd_marathon_perk_f(const command::params& params)
  {
    if (params.size() < 2) return;

    const auto player_num = std::atoi(params.get(1));
    auto* const g_client = game::g_entities[player_num].client;

    if (g_client == nullptr) return;

    g_client->ps.perks[0] ^= 0x4000u;
  }

  void cmd_force_last_stand_f(const command::params& params)
  {
    if (params.size() < 2) return;

    const auto player_num = std::atoi(params.get(1));
    auto* g_client = game::g_entities[player_num].client;

    if (g_client == nullptr) return;

    g_client->lastStand = 1;
    g_client->lastStandTime = 0;
  }

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      register_dvars();

      utils::hook::call(0x41F995, pm_trace_stub);
      utils::hook::call(0x41F8D8, pm_trace_stub);
      utils::hook::call(0x41F941, pm_trace_stub);

      utils::hook::call(0x422861, pm_player_trace_stub);
      utils::hook::call(0x4228B5, pm_player_trace_stub);

      utils::hook::call(0x41E210, pm_move_scale_stub);
      utils::hook::call(0x41E48D, pm_move_scale_stub);
      utils::hook::call(0x4227B2, pm_move_scale_stub);

      utils::hook::call(0x424E0A, pm_project_velocity_stub);

      add_movement_commands();
    }

   private:
    static void add_movement_commands()
    {
      command::add("forceLastStand", cmd_force_last_stand_f);
      command::add("marathonPerk", cmd_marathon_perk_f);
    }

    static void register_dvars()
    {
      bg_elevators = game::Dvar_RegisterBool("bg_elevators",
                                             false,
                                             game::CHEAT | game::CODINFO,
                                             "Elevator glitch settings");

      bg_bouncesAllAngles =
          game::Dvar_RegisterBool("bg_bouncesAllAngles",
                                  false,
                                  game::CHEAT | game::CODINFO,
                                  "Force bounce from all angles");

      cg_noclipScaler =
          game::Dvar_RegisterFloat("cg_noclip_scaler",
                                   3.0f,
                                   0.001f,
                                   1000.0f,
                                   game::CHEAT | game::CODINFO,
                                   "The speed at which noclip camera moves");

      cg_ufoScaler =
          game::Dvar_RegisterFloat("cg_ufo_scaler",
                                   6.0f,
                                   0.001f,
                                   1000.0f,
                                   game::CHEAT | game::CODINFO,
                                   "The speed at which ufo camera moves");
    }
  };
} // namespace movement

REGISTER_COMPONENT(movement::component)
