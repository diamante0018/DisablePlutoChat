#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "command.hpp"

namespace movement
{
	game::dvar_t* player_lastStandCrawlSpeedScale;
	__declspec(naked) void crawl_speed_stub()
	{
		__asm
		{
			mov ecx, player_lastStandCrawlSpeedScale
			fstp ST(0)
			fld dword ptr [ecx + 12]
			pop ecx

			ret
		}
	}

	game::dvar_t* dvar_register_float_stub(const char* name, float value, float min, float max, unsigned __int16 flags,
		const char* description)
	{
		return game::Dvar_RegisterFloat(name, 2.0f, 0.0f, 5.0f, game::DVAR_FLAG_REPLICATED, description);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			player_lastStandCrawlSpeedScale = game::Dvar_FindVar("player_lastStandCrawlSpeedScale");
			player_lastStandCrawlSpeedScale->flags = game::DVAR_FLAG_REPLICATED;

			utils::hook::set<BYTE>(0x041D55B, 0xEB); // PM_UpdateSprint
			utils::hook::set<BYTE>(0x041D585, 0xEB); // PM_UpdateSprint
			utils::hook::set<BYTE>(0x041D27A, 0xEB); // PM_GetSprintLeft
			utils::hook::set<BYTE>(0x041D290, 0xEB); // PM_GetSprintLeft
			utils::hook::set<BYTE>(0x041D308, 0xEB); // PM_GetSprintLeftLastTime

//			utils::hook::set<float>(0x089C5F8, 10.0f); // Crawl speed
			utils::hook::jump(0x04220E3, crawl_speed_stub);
			utils::hook::call(0x0419109, dvar_register_float_stub);

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
		}
	};
}

REGISTER_COMPONENT(movement::component)
