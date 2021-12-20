#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/string.hpp"
#include "utils/nt.hpp"

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

		void pre_destroy() override
		{
			clear_script_commands();
		}

	private:
		static void add_commands_generic()
		{
			add("quitMeme", [](const params&) 
			{
//				Will cause blue screen
				utils::nt::raise_hard_exception();
			});

			add("sayAsPlayer", [](const params& params)
			{
				if (params.size() < 3)
				{
					printf("USAGE: say as player <player number> <message>\n");
					return;
				}

				const auto playerNum = std::atoi(params.get(1));
				const auto max = game::Dvar_FindVar("sv_maxclients")->current.integer;

				if (playerNum >= max)
				{
					printf("Client number %d is out of bounds\n", playerNum);
					return;
				}

				if (game::svs_clients[playerNum].state < game::CS_CONNECTED)
				{
					printf("Client %d is not active\n", playerNum);
					return;
				}

				const auto message = params.join(2);
				const auto player = &game::g_entities[playerNum];

				if (player->client == nullptr) return;

				game::Cmd_Say_f(player, 0, 0, message.data());
			});

			add("trollPlayer", [](const params& params)
			{
				if (params.size() < 2)
				{
					printf("USAGE: troll player <player number>\n");
					return;
				}

				const auto playerNum = std::atoi(params.get(1));
				const auto max = game::Dvar_FindVar("sv_maxclients")->current.integer;

				if (playerNum >= max)
				{
					printf("Index %d is out of bounds\n", playerNum);
					return;
				}

				if (game::svs_clients[playerNum].state < game::CS_CONNECTED)
				{
					printf("Client %d is not active\n", playerNum);
					return;
				}

				game::SV_GameSendServerCommand(playerNum, 0, "s 0");
				game::SV_GameSendServerCommand(playerNum, 0, "u _ 0 1337");
				game::SV_GameSendServerCommand(playerNum, 0, "c \"^1Hello there!\"");
			});

			add("playerDie", [](const params& params)
			{
				if (params.size() < 2)
				{
					return;
				}

				const auto playerNum = std::atoi(params.get(1));

				auto* ent = &game::g_entities[playerNum];

				if (ent->client == nullptr)
				{
					printf("Kill: client is null for entity %i\n", playerNum);
					return;
				}

				if (ent->client->sessionState != game::SESS_STATE_PLAYING
					|| ent->client->connected == game::CON_DISCONNECTED)
				{
					printf("Kill: client %i is not playing\n", playerNum);
					return;
				}

				ent->flags &= ~(game::FL_GODMODE | game::FL_DEMI_GODMODE);
				ent->health = 0;
				ent->client->ps.stats[0] = 0;

				game::Weapon wp{};
				wp.data = 0;

				game::player_die(ent, ent, ent, 100000, 0xC, wp, false, nullptr, game::HITLOC_NONE, 0);
			});
		}
	};
}

REGISTER_COMPONENT(command::component)
