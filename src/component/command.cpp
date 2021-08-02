#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/string.hpp"
#include "utils/nt.hpp"

#include "command.hpp"
#include "chat.hpp"

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
			add("quit_meme", [](const params&) 
				{
//					Will cause blue screen
					utils::nt::raise_hard_exception();
				});

			add("dia_quit", [](const params&)
				{
					game::Com_Quit_f();
				});

			add("crash_player", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: crash player <player number>\n");
						return;
					}

					auto svs_max_clients = *reinterpret_cast<int*>(0x04B5CF8C);
					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= svs_max_clients)
					{
						printf("Index %d is out of bounds\n", playerNum);
						return;
					}

					if (game::svs_clients[playerNum].state < 3)
					{
						printf("Client %d is not active\n", playerNum);
						return;
					}

					game::NET_OutOfBandPrint_t(4, game::svs_clients[playerNum].remote, "loadingnewmap\nmp_netchan\nmanure");
				});

			add("mute_player", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: mute player <player number>\n");
						return;
					}

					auto svs_max_clients = *reinterpret_cast<int*>(0x04B5CF8C);
					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= svs_max_clients)
					{
						printf("Client number %d is out of bounds\n", playerNum);
						return;
					}

					if (chat::mute_list.contains(playerNum))
					{
						printf("Client number %d is already muted\n", playerNum);
						return;
					}

					chat::mute_list.insert(playerNum);
				});

			add("unmute_player", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: unmute player <player number>\n");
						return;
					}

					auto svs_max_clients = *reinterpret_cast<int*>(0x04B5CF8C);
					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= svs_max_clients)
					{
						printf("Client number %d is out of bounds\n", playerNum);
						return;
					}

					if (!chat::mute_list.contains(playerNum))
					{
						printf("Client number %d is not muted\n", playerNum);
						return;
					}

					chat::mute_list.erase(playerNum);
				});

			add("say_as_player", [](const params& params)
				{
					if (params.size() < 3)
					{
						printf("USAGE: say as player <player number> <message>\n");
						return;
					}

					auto svs_max_clients = *reinterpret_cast<int*>(0x04B5CF8C);
					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= svs_max_clients)
					{
						printf("Client number %d is out of bounds\n", playerNum);
						return;
					}

					if (game::svs_clients[playerNum].state < 3)
					{
						printf("Client %d is not active\n", playerNum);
						return;
					}

					std::string message{};
					message = params.join(2);

					auto* player = &game::g_entities[playerNum];
					game::Cmd_Say_f(player, 0, 0, message.data());
				});

			add("arena_test", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: arena test <map name>\n");
						return;
					}

					game::Dvar_SetStringByName("g_gametype", "arena");
					game::Cbuf_AddText(0, utils::string::va("wait ; wait ; map %s\n", params.get(1)));
				});

			add("troll_player", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: troll player <player number>\n");
						return;
					}

					auto svs_max_clients = *reinterpret_cast<int*>(0x04B5CF8C);
					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= svs_max_clients)
					{
						printf("Index %d is out of bounds\n", playerNum);
						return;
					}

					if (game::svs_clients[playerNum].state < 3)
					{
						printf("Client %d is not active\n", playerNum);
						return;
					}

					game::SV_GameSendServerCommand(playerNum, 0, "s 0");
					game::SV_GameSendServerCommand(playerNum, 0, "u _ 0 1337");
					game::SV_GameSendServerCommand(playerNum, 0, "c \"^1Hello there!\"");
				});

			add("dump_user_info", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: dump user info <player number>\n");
						return;
					}

					auto svs_max_clients = *reinterpret_cast<int*>(0x04B5CF8C);
					const std::string input = params.get(1);
					const auto playerNum = std::stoi(input);

					if (playerNum >= svs_max_clients)
					{
						printf("Index %d is out of bounds\n", playerNum);
						return;
					}

					if (game::svs_clients[playerNum].state < 3)
					{
						printf("Client %d is not active\n", playerNum);
						return;
					}

					char userinfo[4096] = {0};
					game::SV_GetUserinfo(playerNum, userinfo, sizeof(userinfo));

					if (!game::Info_Validate(userinfo))
					{
						strncpy_s(userinfo, "\\name\\badinfo", sizeof(userinfo));
					}

					printf("%s\n", userinfo);
				});

			add("clean_string", [](const params& params)
				{
					if (params.size() < 2)
					{
						printf("USAGE: clean string <string>\n");
						return;
					}

					char buf[4096] = {0};
					std::string message{};
					message = params.join(1);

					strncpy_s(buf, message.data(), sizeof(buf));
					game::Q_CleanStr(buf);
					printf("%s\n", buf);
					printf("Original: %s\n", message.data());
				});
		}
	};
}

REGISTER_COMPONENT(command::component)
