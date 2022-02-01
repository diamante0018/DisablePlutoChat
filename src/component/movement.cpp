#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "command.hpp"

namespace movement
{
	game::dvar_t* player_lastStandCrawlSpeedScale;
	game::dvar_t* player_duckedSpeedScale;
	game::dvar_t* player_proneSpeedScale;
	game::dvar_t* player_activate_slowdown;
	game::dvar_t* player_spectateSpeedScale;
	game::dvar_t* cg_ufoScaler;
	game::dvar_t* cg_noclipScaler;
	game::dvar_t* bg_bouncesAllAngles;
	game::dvar_t* bg_elevators;

	float pm_cmd_scale_for_stance(const game::pmove_t* pm)
	{
		assert(pm->ps != nullptr);

		float scale{};
		const auto* playerState = pm->ps;

		if (playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0xB)
		{
			scale = pm->cmd.serverTime - playerState->viewHeightLerpTime / 400.0f;
			if (0.0f <= scale)
			{
				if (scale > 1.0f)
				{
					scale = 1.0f;
					return scale * 0.15f + (1.0f - scale) * 0.65f;
				}

				if (scale != 0.0f)
				{
					return scale * 0.15f + (1.0f - scale) * 0.65f;
				}
			}
		}

		if ((playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0x28) &&
			playerState->viewHeightLerpDown == 0)
		{
			scale = 400.0f / pm->cmd.serverTime - playerState->viewHeightLerpTime;
			if (0.0f <= scale)
			{
				if (scale > 1.0f)
				{
					scale = 1.0f;
				}
				else if (scale != 0.0f)
				{
					return scale * 0.65f + (1.0f - scale) * 0.15f;
				}
			}
		}

		scale = 1.0f;
		const auto stance = game::PM_GetEffectiveStance(playerState);

		if (stance == game::PM_EFF_STANCE_PRONE)
		{
			return player_proneSpeedScale->current.value;
		}

		if (stance == game::PM_EFF_STANCE_DUCKED)
		{
			if ((playerState->pm_flags & 0x4000) == 0 ||
				(playerState->perks[0] & 0x1000u) == 0)
			{
				return player_duckedSpeedScale->current.value;
			}
		}
		if (stance == game::PM_EFF_STANCE_LASTSTANDCRAWL)
		{
			return player_lastStandCrawlSpeedScale->current.value;
		}

		return scale;
	}

	__declspec(naked) void  pm_cmd_scale_for_stance_stub()
	{
		__asm
		{
			pushad

			push edx
			call pm_cmd_scale_for_stance
			add esp, 4

			popad

			ret
		}
	}

	float pm_move_scale(game::playerState_s* ps, float forwardmove,
		float rightmove, float upmove)
	{
		assert(ps != nullptr);

		auto max = (std::fabsf(forwardmove) < std::fabsf(rightmove))
			? std::fabsf(rightmove)
			: std::fabsf(forwardmove);

		if (std::fabsf(upmove) > max)
		{
			max = std::fabsf(upmove);
		}

		if (max == 0.0f)
		{
			return 0.0f;
		}

		auto total = std::sqrtf(forwardmove * forwardmove
			+ rightmove * rightmove + upmove * upmove);
		auto scale = (ps->speed * max) / (127.0f * total);

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

	void pm_project_velocity_stub(const float* velIn, const float* normal, float* velOut)
	{
		const auto lengthSquared2D = velIn[0] * velIn[0] + velIn[1] * velIn[1];

		if (std::fabsf(normal[2]) < 0.001f || lengthSquared2D == 0.0)
		{
			velOut[0] = velIn[0];
			velOut[1] = velIn[1];
			velOut[2] = velIn[2];
			return;
		}

		auto newZ = velIn[0] * normal[0] + velIn[1] * normal[1];
		newZ = -newZ / normal[2];
		const auto lengthScale = std::sqrtf((velIn[2] * velIn[2] + lengthSquared2D)
			/ (newZ * newZ + lengthSquared2D));

		if (bg_bouncesAllAngles->current.enabled
			|| (lengthScale < 1.f || newZ < 0.f || velIn[2] > 0.f))
		{
			velOut[0] = velIn[0] * lengthScale;
			velOut[1] = velIn[1] * lengthScale;
			velOut[2] = newZ * lengthScale;
		}
	}

	void jump_activate_slowdown_stub(game::playerState_s* ps)
	{
		if (player_activate_slowdown->current.enabled)
		{
			reinterpret_cast<void (*)(game::playerState_s*)>(0x04161C0)(ps);
		}
	}

	int is_prone_allowed_stub()
	{
		return TRUE;
	}

	void pm_trace_stub(game::pmove_t* move, game::trace_t* trace, const float* f3, const float* f4,
		const game::Bounds* bounds, int a6, int a7)
	{
		game::PM_trace(move, trace, f3, f4, bounds, a6, a7);

		if (bg_elevators->current.enabled)
		{
			trace->allsolid = false;
		}
	}

	void pm_player_trace_stub(game::pmove_t* move, game::trace_t* trace, const float* f3,
		const float* f4, const game::Bounds* bounds, int a6, int a7)
	{
		game::PM_playerTrace(move, trace, f3, f4, bounds, a6, a7);

		if (bg_elevators->current.enabled)
		{
			trace->startsolid = false;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			register_dvars();

			utils::hook::call(0x042228B, pm_cmd_scale_for_stance_stub);
			utils::hook::call(0x0422D16, pm_cmd_scale_for_stance_stub);
			utils::hook::call(0x0422D3F, pm_cmd_scale_for_stance_stub);

			utils::hook::call(0x041FA40, jump_activate_slowdown_stub);

			utils::hook::call(0x041F793, is_prone_allowed_stub);

			utils::hook::set<BYTE>(0x04F9F39, 0x75); // ClientEndFrame

			utils::hook::call(0x041F995, pm_trace_stub);

			utils::hook::call(0x0422861, pm_player_trace_stub);
			utils::hook::call(0x04228B5, pm_player_trace_stub);

			utils::hook::call(0x041E210, pm_move_scale_stub);
			utils::hook::call(0x041E48D, pm_move_scale_stub);

			utils::hook::call(0x04227B2, pm_move_scale_stub);

			add_movement_commands();
		}

	private:
		static void add_movement_commands()
		{
			command::add("force_last_stand", [](const command::params& params)
			{
				if (params.size() < 2) return;

				auto* g_client = game::g_entities[std::atoi(params.get(1))].client;

				if (g_client == nullptr) return;

				g_client->lastStand = 1;
				g_client->lastStandTime = 0;
			});

			command::add("marathonPerk", [](const command::params& params)
			{
				if (params.size() < 2) return;

				auto* g_client = game::g_entities[std::atoi(params.get(1))].client;

				if (g_client == nullptr) return;

				auto playerState = &g_client->ps;
				playerState->perks[0] ^= 0x4000u;
			});
		}

		static void register_dvars()
		{
			player_lastStandCrawlSpeedScale = game::Dvar_FindVar("player_lastStandCrawlSpeedScale");
			player_spectateSpeedScale = game::Dvar_FindVar("player_spectateSpeedScale");

			player_duckedSpeedScale = game::Dvar_RegisterFloat("player_duckedSpeedScale", 0.65f, 0.0f, 5.0f,
				game::CHEAT | game::CODINFO,
				"The scale applied to the player speed when ducking");
			player_proneSpeedScale = game::Dvar_RegisterFloat("player_proneSpeedScale", 0.15f, 0.0f, 5.0f,
				game::CHEAT | game::CODINFO,
				"The scale applied to the player speed when crawling");
			player_activate_slowdown = game::Dvar_RegisterBool("player_activate_slowdown", true,
				game::CHEAT | game::CODINFO,
				"Slow the player down");

			bg_elevators = game::Dvar_RegisterBool("bg_elevators", false,
				game::CHEAT | game::CODINFO,
				"Elevator glitch settings");
			bg_bouncesAllAngles = game::Dvar_RegisterBool("bg_bouncesAllAngles", false,
				game::CHEAT | game::CODINFO,
				"Force bounce from all angles");

			cg_noclipScaler = game::Dvar_RegisterFloat("cg_noclip_scaler", 3.0f, 0.001f, 1000.0f,
				game::CHEAT | game::CODINFO,
				"The speed at which noclip camera moves");
			cg_ufoScaler = game::Dvar_RegisterFloat("cg_ufo_scaler", 6.0f, 0.001f, 1000.0f,
				game::CHEAT | game::CODINFO,
				"The speed at which ufo camera moves");
		}
	};
}

REGISTER_COMPONENT(movement::component)
