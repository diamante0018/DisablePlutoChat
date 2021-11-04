#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

namespace collision
{
	game::dvar_t* g_playerCollision;
	__declspec(naked) void player_collision_stub()
	{
		__asm
		{
			push eax
			mov eax, g_playerCollision
			cmp byte ptr [eax + 12], 0
			pop eax

			je dontcollide

			mov eax, dword ptr [esp + 0xa0]
			push 0x05413B6
			retn

		dontcollide:
			mov eax, dword ptr [esp + 0xa0]
			mov ecx, dword ptr [esp + 9ch]
			push eax
			push ecx
			lea edx, [esp + 48h]
			push edx
			mov eax, esi
			push 0x05413CB
			retn
		}
	}

	game::dvar_t* g_playerEjection;
	__declspec(naked) void player_ejection_stub()
	{
		__asm
		{
			push eax
			mov eax, g_playerEjection
			cmp byte ptr [eax + 12], 0
			pop eax

			je donteject

			push 0x04F9EFA
			retn

		donteject:
			push 0x04F9F03
			retn
		}
	}

	game::dvar_t* sv_enableBounces_plug;
	__declspec(naked) void bounce_stub()
	{
		__asm
		{
//			Check the value of sv_enableBounces
			push eax
			mov eax, sv_enableBounces_plug
			cmp byte ptr [eax + 12], 1
			pop eax

//			Always bounce if sv_enableBounces is set to 1
			je bounce

//			Original code
			cmp dword ptr [esp + 24h], 0
			jnz dontBounce

		bounce:
			push 0x0424D58
			retn

		dontBounce:
			push 0x0424D6C
			retn
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			g_playerCollision = game::Dvar_RegisterBool("g_playerCollision", true, game::DVAR_FLAG_NONE, "Flag whether player collision is on or off");
			g_playerEjection = game::Dvar_RegisterBool("g_playerEjection", true, game::DVAR_FLAG_NONE, "Flag whether player ejection is on or off");
			sv_enableBounces_plug = game::Dvar_RegisterBool("sv_enableBounces_plug", true, game::DVAR_FLAG_NONE, "Enables bouncing on the server");

			utils::hook::jump(0x05413AF, player_collision_stub);
			utils::hook::jump(0x04F9EF2, player_ejection_stub);
			utils::hook::jump(0x0424D51, bounce_stub);

			utils::hook::set<BYTE>(0x061915F, 0xEB); // Enable elevators, I think
			utils::hook::set<BYTE>(0x042286D, 0xEB);
		}
	};
}

REGISTER_COMPONENT(collision::component)
