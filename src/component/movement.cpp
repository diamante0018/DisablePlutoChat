#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

namespace movement
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			game::dvar_t* player_sprintUnlimited = game::Dvar_FindVar("player_sprintUnlimited");
			player_sprintUnlimited->flags = game::DVAR_FLAG_REPLICATED; // Arrest me. Removed cheat flag and two other flags I don't care about

			utils::hook::set<BYTE>(0x041D55B, 0xEB);
			utils::hook::set<BYTE>(0x041D585, 0xEB);
		}
	};
}

REGISTER_COMPONENT(movement::component)
