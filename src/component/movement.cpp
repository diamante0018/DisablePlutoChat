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

	__declspec(naked) void pm_jitter_point_stub_1()
	{
		__asm
		{
			push eax
			mov eax, bg_elevators
			cmp byte ptr [eax + 12], 1
			pop eax

			je skipJump

			cmp byte ptr [ebp + 0x29], 0
			jnz originalCode

		skipJump:
			push 0x042286F
			retn

		originalCode:
			push 0x04228C3
			retn
		}
	}

	__declspec(naked) void pm_jitter_point_stub_2()
	{
		__asm
		{
			push eax
			mov eax, bg_elevators
			cmp byte ptr [eax + 12], 1
			pop eax

			je jumpOver

			cmp byte ptr [ebp + 0x29], 0
			jz jumpOver

			push 0x04228C3
			retn

		jumpOver:
			push 0x04228E6
			retn
		}
	}

	void pm_trace_stub(game::pmove_t* move, game::trace_t* trace, const float* const f3, const float* const f4,
		const game::Bounds* bounds, int a6, int a7)
	{
		if (!bg_elevators->current.enabled)
		{
			reinterpret_cast<void (*)(game::pmove_t*, game::trace_t*, const float* const,
				const float* const, const game::Bounds*, int, int)>(0x0421F00)(move, trace, f3, f4, bounds, a6, a7);
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

			utils::hook::call(0x041F83C, pm_trace_stub);
			utils::hook::call(0x041F941, pm_trace_stub);
			utils::hook::call(0x041F995, pm_trace_stub);
			utils::hook::call(0x041F8D8, pm_trace_stub);
			utils::hook::call(0x041F995, pm_trace_stub);

			utils::hook::jump(0x0422869, pm_jitter_point_stub_1); // I don't know
			utils::hook::nop(0x042286E, 1);

			utils::hook::jump(0x04228BD, pm_jitter_point_stub_2); // IW4 code flow
			utils::hook::nop(0x04228C2, 1);

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

			command::add("marathon_perk", [](const command::params& params)
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
		}
	};
}

REGISTER_COMPONENT(movement::component)
