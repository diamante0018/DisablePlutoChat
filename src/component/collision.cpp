#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "collision.hpp"

namespace collision
{
	__declspec(naked) void player_collision_stub()
	{
		__asm
		{
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

	__declspec(naked) void player_ejection_stub()
	{
		__asm
		{
			push 0x04F9F03;
			retn;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			utils::hook::jump(0x05413AF, player_collision_stub);
			utils::hook::jump(0x04F9EF2, player_ejection_stub);
		}
	};
}

REGISTER_COMPONENT(collision::component)
