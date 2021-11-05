#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "scheduler.hpp"

namespace movement
{
	game::dvar_t* player_sprintUnlimited;
	__declspec(naked) void pm_end_sprint_dvar_stub()
	{
		__asm
		{
			push eax
			mov eax, player_sprintUnlimited
			cmp byte ptr [eax + 12], 1
			pop eax

			je get_out

			push 0x041D350
			retn

		get_out:
			ret
		}
	}

	game::dvar_t* dvar_register_bool_stub(const char* name, bool, unsigned __int16, const char* description)
	{
		return game::Dvar_RegisterBool(name, true, game::DVAR_FLAG_REPLICATED, description);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			player_sprintUnlimited = game::Dvar_FindVar("player_sprintUnlimited");
			player_sprintUnlimited->flags = game::DVAR_FLAG_REPLICATED; // Arrest me. Removed cheat flag and more

			utils::hook::jump(0x041D565, pm_end_sprint_dvar_stub);
			utils::hook::call(0x041D58B, pm_end_sprint_dvar_stub);

			utils::hook::call(0x04190C0, dvar_register_bool_stub);
		}
	};
}

REGISTER_COMPONENT(movement::component)
