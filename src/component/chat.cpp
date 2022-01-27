#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "utils/string.hpp"

#include "command.hpp"

namespace chat
{
    std::mutex access_mutex;
    static std::unordered_set<std::int32_t> mute_list{};
    game::dvar_t* sv_enableGameChat;

    void client_command_stub(int clientNum)
    {
        char cmd[1024]{};
        game::gentity_s* ent = &game::g_entities[clientNum];
        if (ent->client == nullptr)
        {
//          Not fully in game yet
            return;
        }

        game::SV_Cmd_ArgvBuffer(0, cmd, sizeof(cmd));

        if (utils::string::starts_with(cmd, "say"))
        {
            std::unique_lock<std::mutex> _(access_mutex);
            if (!sv_enableGameChat->current.enabled || mute_list.contains(clientNum))
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

            sv_enableGameChat = game::Dvar_RegisterBool("sv_enableGameChat", true, game::SERVERINFO, "Enable game chat");
        }

    private:
        static void add_chat_commands()
        {
            command::add("mutePlayer", [](const command::params& params)
            {
                if (params.size() < 2)
                {
                    return;
                }

                const auto input = params.get(1);
                const auto playerNum = std::atoi(input);

                if (playerNum >= *game::svs_clientCount)
                {
                    return;
                }

                std::unique_lock<std::mutex> _(access_mutex);
                if (mute_list.contains(playerNum))
                {
                    return;
                }

                mute_list.insert(playerNum);
            });

            command::add("unmutePlayer", [](const command::params& params)
            {
                if (params.size() < 2)
                {
                    return;
                }

                const auto input = params.get(1);
                const auto playerNum = std::atoi(input);

                if (playerNum >= *game::svs_clientCount)
                {
                    return;
                }

                std::unique_lock<std::mutex> _(access_mutex);
                if (!mute_list.contains(playerNum))
                {              
                    return;
                }

                mute_list.erase(playerNum);
            });
        }
    };
}

REGISTER_COMPONENT(chat::component)
