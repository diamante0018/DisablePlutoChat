#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "dvars.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace dvars
{
	game::dvar_t* sv_EnableGameChat;

	game::dvar_t* Dvar_RegisterBool(const std::string &name, const bool value, const unsigned short flags,
						const std::string &description)
	{
		return game::Dvar_RegisterBool(name.data(), value, flags, description.data());
	}

	game::dvar_t* Dvar_RegisterFloat(const std::string &name, const float value, const float min, const float max,
						const unsigned short flags,
						const std::string &description)
	{
		return game::Dvar_RegisterFloat(name.data(), value, min, max, flags, description.data());
	}

	game::dvar_t* Dvar_RegisterInt(const std::string &name, const int value, const int min, const int max,
						const unsigned short flags,
						const std::string &description)
	{
		game::Dvar_RegisterInt(name.data(), value, min, max, flags, description.data());
	}

	game::dvar_t* Dvar_RegisterString(const std::string &name, const std::string &value, const unsigned short flags,
						const std::string &description)
	{
		game::Dvar_RegisterString(name.data(), value.data(), flags, description.data());
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			sv_EnableGameChat = Dvar_RegisterBool("sv_EnableGameChat", true, game::DVAR_FLAG_NONE, "Enables global and team chat");
		}
	};
}

REGISTER_COMPONENT(dvars::component)
