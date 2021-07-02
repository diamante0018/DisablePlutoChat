#pragma once

namespace dvars
{
	extern game::dvar_t* sv_EnableGameChat;

	namespace override
	{
		void Dvar_RegisterBool(const std::string& name, bool value, const unsigned int flags, const std::string& description = "");
		void Dvar_RegisterFloat(const std::string& name, float value, float min, float max, const unsigned int flags, const std::string& description = "");
		void Dvar_RegisterInt(const std::string& name, int value, int min, int max, const unsigned int flags, const std::string& description = "");
		void Dvar_RegisterString(const std::string& name, const std::string& value, const unsigned int flags, const std::string& description = "");
	}
}
