#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "collision.hpp"

namespace collision
{
	game::dvar_t* g_playerCollision;
	__declspec(naked) void player_collision_stub()
	{
		__asm
		{
			push eax;
			mov eax, g_playerCollision;
			cmp byte ptr[eax + 12], 0;
			pop eax;

			je dontcollide;

			mov eax, dword ptr[esp + 0xa0];
			jmp collide;

		collide:
			push 0x05413B6;
			retn;

		dontcollide:
			mov eax, dword ptr[esp + 0xa0];
			mov ecx, dword ptr[esp + 9ch];
			push eax;
			push ecx;
			lea edx, [esp + 48h];
			push edx;
			mov eax, esi;
			push 0x05413CB;
			retn;
		}
	}

	game::dvar_t* g_playerEjection;
	__declspec(naked) void player_ejection_stub()
	{
		__asm
		{
			push eax;
			mov eax, g_playerEjection;
			cmp byte ptr[eax + 12], 0;
			pop eax;

			je donteject;

			cmp dword ptr[ebx + 19ch], edi;
			jle eject;

		eject:
			push 0x04F9EFA;
			retn;

		donteject:
			push 0x04F9F03;
			retn;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			g_playerCollision = game::Dvar_RegisterBool("g_playerCollision", true, game::DVAR_FLAG_NONE, "Flag whether player collision is on or off");
			g_playerEjection = game::Dvar_RegisterBool("g_playerEjection", true, game::DVAR_FLAG_NONE, "Flag whether player ejection is on or off");

			utils::hook::jump(0x05413AF, player_collision_stub);
			utils::hook::jump(0x04F9EF2, player_ejection_stub);
		}
	};
}

REGISTER_COMPONENT(collision::component)
