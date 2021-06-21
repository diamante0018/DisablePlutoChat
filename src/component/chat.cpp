#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "utils/string.hpp"

#include "chat.hpp"

namespace chat
{
    namespace
    {
        utils::hook::detour client_ui_hook;

        void client_ui_stub(int entRef, const char* command)
        {
            const char* debug = utils::string::va("Client: %d sent command: %s\n", entRef, command);
            game::Cbuf_AddText(0, debug);

            if (utils::string::starts_with(command, "say")) return;

            client_ui_hook.invoke<void>(entRef, command);
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            client_ui_hook.create(0x047EA40, &client_ui_stub);
        }
    };
}

REGISTER_COMPONENT(chat::component)
