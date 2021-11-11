#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include <utils/hook.hpp>

#include "dvars.hpp"
#include "game/game.hpp"

namespace dvars
{
	struct dvar_base
	{
		unsigned short flags{};
		std::string description{};
	};

	struct dvar_bool : dvar_base
	{
		bool value{};
	};

	struct dvar_float : dvar_base
	{
		float value{};
		float min{};
		float max{};
	};

	struct dvar_int : dvar_base
	{
		int value{};
		int min{};
		int max{};
	};

	struct dvar_string : dvar_base
	{
		std::string value{};
	};

	namespace
	{
		template <typename T>
		T* find_dvar(std::unordered_map<std::string, T>& map, const std::string& name)
		{
			auto i = map.find(name);
			if (i != map.end())
			{
				return &i->second;
			}

			return nullptr;
		}

		bool find_dvar(std::unordered_set<std::string>& set, const std::string& name)
		{
			return set.find(name) != set.end();
		}
	}

	namespace override
	{
		static std::unordered_map<std::string, dvar_bool> register_bool_overrides;
		static std::unordered_map<std::string, dvar_float> register_float_overrides;
		static std::unordered_map<std::string, dvar_int> register_int_overrides;
		static std::unordered_map<std::string, dvar_string> register_string_overrides;

		void Dvar_RegisterBool(const std::string& name, const bool value, const unsigned short flags,
		                       const std::string& description)
		{
			dvar_bool values;
			values.value = value;
			values.flags = flags;
			values.description = description;
			register_bool_overrides[name] = std::move(values);
		}

		void Dvar_RegisterFloat(const std::string& name, const float value, const float min, const float max,
		                        const unsigned short flags,
		                        const std::string& description)
		{
			dvar_float values;
			values.value = value;
			values.min = min;
			values.max = max;
			values.flags = flags;
			values.description = description;
			register_float_overrides[name] = std::move(values);
		}

		void Dvar_RegisterInt(const std::string& name, const int value, const int min, const int max,
		                      const unsigned short flags,
		                      const std::string& description)
		{
			dvar_int values;
			values.value = value;
			values.min = min;
			values.max = max;
			values.flags = flags;
			values.description = description;
			register_int_overrides[name] = std::move(values);
		}

		void Dvar_RegisterString(const std::string& name, const std::string& value, const unsigned short flags,
		                         const std::string& description)
		{
			dvar_string values;
			values.value = value;
			values.flags = flags;
			values.description = description;
			register_string_overrides[name] = std::move(values);
		}
	}

	utils::hook::detour dvar_register_bool_hook;
	utils::hook::detour dvar_register_float_hook;
	utils::hook::detour dvar_register_int_hook;
	utils::hook::detour dvar_register_string_hook;

	game::dvar_t* dvar_register_bool(const char* name, bool value, unsigned short flags, const char* description)
	{
		auto* var = find_dvar(override::register_bool_overrides, name);
		if (var)
		{
			value = var->value;
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_bool_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	game::dvar_t* dvar_register_float(const char* name, float value, float min, float max, unsigned short flags,
	                                  const char* description)
	{
		auto* var = find_dvar(override::register_float_overrides, name);
		if (var)
		{
			value = var->value;
			min = var->min;
			max = var->max;
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_float_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_int(const char* name, int value, int min, int max, unsigned short flags,
	                                const char* description)
	{
		auto* var = find_dvar(override::register_int_overrides, name);
		if (var)
		{
			value = var->value;
			min = var->min;
			max = var->max;
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_int_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_string(const char* name, const char* value, unsigned short flags, const char* description)
	{
		auto* var = find_dvar(override::register_string_overrides, name);
		if (var)
		{
			value = var->value.data();
			flags = var->flags;
			description = var->description.data();
		}

		return dvar_register_string_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			dvar_register_bool_hook.create(0x05BE9F0, &dvar_register_bool);
			dvar_register_float_hook.create(0x05BEA80, &dvar_register_float);
			dvar_register_int_hook.create(0x05BEA40, &dvar_register_int);
			dvar_register_string_hook.create(0x05BEC90, &dvar_register_string);
		}

		void pre_destroy() override
		{
			dvar_register_bool_hook.clear();
			dvar_register_float_hook.clear();
			dvar_register_int_hook.clear();
			dvar_register_string_hook.clear();
		}
	};
}

REGISTER_COMPONENT(dvars::component)
