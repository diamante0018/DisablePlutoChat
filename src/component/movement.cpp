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
	game::dvar_t* sv_enableElevators;

	int pm_get_effective_stance(const game::playerState_s* ps) // Inlined on IW5
	{
		const auto heightTarget = ps->viewHeightTarget;
		if (heightTarget == 0x16)
		{
			return game::PM_EFF_STANCE_LASTSTANDCRAWL;
		}

		if (heightTarget == 0x28)
		{
			return game::PM_EFF_STANCE_DUCKED;
		}

		if (heightTarget == 0xB)
		{
			return game::PM_EFF_STANCE_PRONE;
		}

		return game::PM_EFF_STANCE_DEFAULT;
	}

	float pm_cmd_scale_for_stance(game::pmove_t* move)
	{
		float scale{};
		const auto* playerState = move->ps;

		if (playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0xB)
		{
			scale = move->cmd.serverTime - playerState->viewHeightLerpTime / 400.0f;
			if (0.0f <= scale)
			{
				auto flags = 0;
				if (scale < 1.0f)
				{
					flags |= 1 << 8;
				}
				if (scale == 1.0f)
				{
					flags |= 1 << 14;
				}

				if (flags == 0)
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
			scale = 400.0f / move->cmd.serverTime - playerState->viewHeightLerpTime;
			if (0.0f <= scale)
			{
				auto flags = 0;
				if (scale < 1.0f)
				{
					flags |= 1 << 8;
				}

				if (scale == 1.0f)
				{
					flags |= 1 << 14;
				}

				if (flags == 0)
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
		auto stance = pm_get_effective_stance(playerState);

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

		if (sv_enableElevators->current.enabled)
		{
			trace->allsolid = false;
		}
	}

	void pm_player_trace_stub(game::pmove_t* move, game::trace_t* trace, const float* f3,
		const float* f4, const game::Bounds* bounds, int a6, int a7)
	{
		game::PM_playerTrace(move, trace, f3, f4, bounds, a6, a7);

		if (sv_enableElevators->current.enabled)
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

			add_movement_commands();
		}

	private:
		static void add_movement_commands()
		{
			command::add("teleportPlayer", [](const command::params& params)
			{
				if (params.size() < 5) return;

				const auto ent = &game::g_entities[std::atoi(params.get(1))];
				game::vec3_t newOrigin{};
				game::vec3_t neutralViewAngle{ 0.0f, 0.0f, 0.0f };
				newOrigin[0] = std::stof(params.get(2));
				newOrigin[1] = std::stof(params.get(3));
				newOrigin[2] = std::stof(params.get(4));

				game::TeleportPlayer(ent, newOrigin, neutralViewAngle);
			});

			command::add("forceLastStand", [](const command::params& params)
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

				const auto g_client = game::g_entities[std::atoi(params.get(1))].client;

				if (g_client == nullptr) return;

				auto playerState = &g_client->ps;
				playerState->perks[0] ^= 0x4000u;
			});
		}

		static void register_dvars()
		{
			player_lastStandCrawlSpeedScale = game::Dvar_FindVar("player_lastStandCrawlSpeedScale");
			player_duckedSpeedScale = game::Dvar_RegisterFloat("player_duckedSpeedScale", 0.65f, 0.0f, 5.0f,
				game::DVAR_FLAG_CHEAT | game::DVAR_FLAG_REPLICATED,
				"The scale applied to the player speed when ducking");
			player_proneSpeedScale = game::Dvar_RegisterFloat("player_proneSpeedScale", 0.15f, 0.0f, 5.0f,
				game::DVAR_FLAG_CHEAT | game::DVAR_FLAG_REPLICATED,
				"The scale applied to the player speed when crawling");
			player_activate_slowdown = game::Dvar_RegisterBool("player_activate_slowdown", true,
				game::DVAR_FLAG_CHEAT | game::DVAR_FLAG_REPLICATED,
				"Slow the player down");
			sv_enableElevators = game::Dvar_RegisterBool("sv_enableElevators", false,
				game::DVAR_FLAG_CHEAT | game::DVAR_FLAG_REPLICATED,
				"sv_enableElevators");
		}
	};
}

REGISTER_COMPONENT(movement::component)
