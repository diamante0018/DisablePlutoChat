#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "command.hpp"

namespace movement
{
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::set<BYTE>(0x041D55B, 0xEB); // PM_UpdateSprint
			utils::hook::set<BYTE>(0x041D585, 0xEB); // PM_UpdateSprint
			utils::hook::set<BYTE>(0x041D27A, 0xEB); // PM_GetSprintLeft
			utils::hook::set<BYTE>(0x041D290, 0xEB); // PM_GetSprintLeft
			utils::hook::set<BYTE>(0x041D308, 0xEB); // PM_GetSprintLeftLastTime

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
		}
	};
}

REGISTER_COMPONENT(movement::component)
