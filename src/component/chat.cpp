#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "utils/string.hpp"

#include "command.hpp"

namespace chat
{
    std::unordered_set<std::int32_t> mute_list{};
    game::dvar_t* sv_EnableGameChat;

    void client_command_stub(int clientNum)
    {
        char cmd[1024] = {0};
        game::gentity_s* ent = &game::g_entities[clientNum];
        if (ent->client == nullptr)
        {
//          Not fully in game yet
            return;
        }

        game::SV_Cmd_ArgvBuffer(0, cmd, sizeof(cmd));

        if (utils::string::starts_with(cmd, "die"))
        {
            game::Dvar_SetStringByName("sv_iw4madmin_command", utils::string::va("kill;%d", clientNum));
            game::Cbuf_InsertText(0, utils::string::va("tell %d \"You have commited suicide\"", clientNum));
            return;
        }

        if (utils::string::starts_with(cmd, "say"))
        {
            if (!sv_EnableGameChat->current.enabled || mute_list.contains(clientNum))
            {
                game::Cbuf_InsertText(0, utils::string::va("tell %d \"You are not allowed to type in the chat\"", clientNum));
                return;
            }
        }

        reinterpret_cast<void (*)(int)>(0x0502CB0)(clientNum);
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            utils::hook::call(0x057192A, client_command_stub);
            add_chat_commands();

            sv_EnableGameChat = game::Dvar_RegisterBool("sv_EnableGameChat", true, game::DVAR_FLAG_NONE, "Enables global and team chat");
        }

        void pre_destroy() override
        {
            mute_list.clear();
        }

    private:
        static void add_chat_commands()
        {
            command::add("mute_player", [](const command::params& params)
            {
                if (params.size() < 2)
                {
                    printf("USAGE: mute player <player number>\n");
                    return;
                }

                const auto input = params.get(1);
                const auto playerNum = std::atoi(input);
                const auto max = game::Dvar_FindVar("sv_maxclients")->current.integer;

                if (playerNum >= max)
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

            command::add("unmute_player", [](const command::params& params)
            {
                if (params.size() < 2)
                {
                    printf("USAGE: unmute player <player number>\n");
                    return;
                }

                const auto input = params.get(1);
                const auto playerNum = std::atoi(input);
                const auto max = game::Dvar_FindVar("sv_maxclients")->current.integer;

                if (playerNum >= max)
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
        }
    };
}

REGISTER_COMPONENT(chat::component)
