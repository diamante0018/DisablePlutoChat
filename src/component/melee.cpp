#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

namespace melee
{
    game::dvar_t* g_allowMelee;
    void fire_melee_stub(game::gentity_s* ent, int a2)
    {
        if (g_allowMelee->current.enabled)
        {
            reinterpret_cast<void (*)(game::gentity_s*, int)>(0x0530DB0)(ent, a2);
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            g_allowMelee = game::Dvar_RegisterBool("g_allowMelee", true, game::DVAR_FLAG_NONE, "Allows the usage of the knife");
            utils::hook::call(0x04F7AB9, fire_melee_stub);
        }
    };
}

REGISTER_COMPONENT(melee::component)
