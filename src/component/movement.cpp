#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "command.hpp"

namespace movement
{
	game::dvar_t* player_lastStandCrawlSpeedScale;
	game::dvar_t* player_duckedSpeedScale;
	game::dvar_t* player_proneSpeedScale;

	int pm_get_effective_stance(game::playerState_s* ps) // Inlined on IW5
	{
		auto heightTarget = ps->viewHeightTarget;
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
		auto* playerState = move->ps;

		if (playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0xB)
		{
			scale = move->cmd.serverTime - playerState->viewHeightLerpTime / 400.0f;
			if (0.0f <= scale)
			{
				auto result = (scale < 1.0f) << 8 | (scale == 1.0f) << 0xE;
				if (result == 0)
				{
					scale = 1.0f;
					return scale * 0.15f + (1.0f - scale) * 0.65f;
				}

				if (scale != 0.0f) return scale * 0.15f + (1.0f - scale) * 0.65f;
			}
		}

		if ((playerState->viewHeightLerpTime != 0 && playerState->viewHeightLerpTarget == 0x28) &&
			playerState->viewHeightLerpDown == 0)
		{
			scale = 400.0f / move->cmd.serverTime - playerState->viewHeightLerpTime;
			if (0.0f <= scale)
			{
				auto result = (scale < 1.0f) << 8 | (scale == 1.0f) << 0xE;
				if (result == 0)
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
				(*(unsigned int*)(playerState->__pad0 + 0x2D0) & 0x1000u) == 0)
			{
				return player_duckedSpeedScale->current.value;
			}
		}
		else if (stance == game::PM_EFF_STANCE_LASTSTANDCRAWL)
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

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			register_dvars();

			utils::hook::call(0x042228B, pm_cmd_scale_for_stance_stub);
			utils::hook::call(0x0422D16, pm_cmd_scale_for_stance_stub);
			utils::hook::call(0x0422D3F, pm_cmd_scale_for_stance_stub);

			utils::hook::set<BYTE>(0x04F9F39, 0x75); // ClientEndFrame

			add_movement_commands();
		}
	private:
		static void add_movement_commands()
		{
			command::add("teleport_player", [](const command::params& params)
			{
				if (params.size() < 5) return;

				auto ent = &game::g_entities[std::stoi(params.get(1))];
				game::vec3_t newOrigin{};
				game::vec3_t neutralViewAngle{ 0.0f, 0.0f, 0.0f };
				newOrigin[0] = std::stof(params.get(2));
				newOrigin[1] = std::stof(params.get(3));
				newOrigin[2] = std::stof(params.get(4));

				game::TeleportPlayer(ent, newOrigin, neutralViewAngle);
			});

			command::add("force_last_stand", [](const command::params& params)
			{
				if (params.size() < 2) return;

				auto g_client = game::g_entities[std::stoi(params.get(1))].client;
				g_client->lastStand = 1;
				g_client->lastStandTime = 0;
			});

			command::add("set_unlimited_sprint", [](const command::params&)
			{
				printf("Warning! This is permanent\n");
				unlimited_sprint();
			});
		}

		static void unlimited_sprint()
		{
			utils::hook::set<BYTE>(0x041D55B, 0xEB); // PM_UpdateSprint
			utils::hook::set<BYTE>(0x041D585, 0xEB); // PM_UpdateSprint
			utils::hook::set<BYTE>(0x041D27A, 0xEB); // PM_GetSprintLeft
			utils::hook::set<BYTE>(0x041D290, 0xEB); // PM_GetSprintLeft
			utils::hook::set<BYTE>(0x041D308, 0xEB); // PM_GetSprintLeftLastTime
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
		}
	};
}

REGISTER_COMPONENT(movement::component)
