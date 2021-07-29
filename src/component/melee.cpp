#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

#include "melee.hpp"

namespace melee
{
	utils::hook::detour fire_melee_hook;

	game::dvar_t* g_allowMelee;
	void fire_melee_stub(game::gentity_s* ent, int a2)
	{
		if (!g_allowMelee->current.enabled) return;

		fire_melee_hook.invoke<void>(ent, a2);
	}

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            g_allowMelee = game::Dvar_RegisterBool("g_allowMelee", true, game::DVAR_FLAG_NONE, "Allows the usage of the knife");
            fire_melee_hook.create(0x0530DB0, &fire_melee_stub);
        }
    };
}

REGISTER_COMPONENT(melee::component)
