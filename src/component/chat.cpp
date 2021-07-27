#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "utils/string.hpp"

#include "chat.hpp"
#include "dvars.hpp"

namespace chat
{
    utils::hook::detour client_command_hook;
    std::unordered_set<std::int32_t> mute_list;

    void client_command_stub(int clientNum)
    {
        char cmd[4096] = {0};
        game::gentity_s* ent = game::g_entities + clientNum;
        if (!ent->client)
        {
//          Not fully in game yet
            return;
        }

        game::SV_Cmd_ArgvBuffer(0, cmd, sizeof(cmd));

        if (utils::string::starts_with(cmd, "die"))
        {
            game::Dvar_SetStringByName("sv_iw4madmin_command", utils::string::va("kill;%d", clientNum));
            game::Cbuf_AddText(0, utils::string::va("tell %d You have commited suicide", clientNum));
            return;
        }

        if (utils::string::starts_with(cmd, "say") && (!dvars::sv_EnableGameChat->current.enabled || mute_list.contains(clientNum)))
        {
            game::Cbuf_AddText(0, utils::string::va("tell %d You are not allowed to type in the chat", clientNum));
            return;
        }

        client_command_hook.invoke<void>(clientNum);
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            client_command_hook.create(0x0502CB0, &client_command_stub);
        }

        void pre_destroy() override
        {
            mute_list.clear();
            client_command_hook.clear();
        }
    };
}

REGISTER_COMPONENT(chat::component)
