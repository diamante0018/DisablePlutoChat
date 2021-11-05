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
			utils::hook::set<BYTE>(0x041D574, 0xEB);
			utils::hook::set<BYTE>(0x041D308, 0xEB);
			utils::hook::set<BYTE>(0x041D282, 0xEB);
		}
	};
}

REGISTER_COMPONENT(movement::component)
