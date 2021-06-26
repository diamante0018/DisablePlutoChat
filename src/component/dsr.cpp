#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "game/game.hpp"

#include "dsr.hpp"
#include "dvars.hpp"

namespace dsr
{
    namespace
    {
        utils::hook::detour open_dsr_hook;

        bool open_DSR_stub(char *dsr_name)
        {
            printf("Loading %s.dsr\n", dsr_name);
            dvars::Dvar_RegisterString("sv_dsr", dsr_name, game::DVAR_FLAG_NONE, "Last DSR opened by the game");

            return open_dsr_hook.invoke<bool>(dsr_name);
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            open_dsr_hook.create(0x06BB470, &open_DSR_stub);
        }
    };
}

REGISTER_COMPONENT(dsr::component)
