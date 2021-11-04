#include <stdinc.hpp>
#include "loader/component_loader.hpp"
#include "scheduler.hpp"

#include "utils/hook.hpp"

namespace zombie
{
    std::unordered_set<std::int32_t> warn_list{};

    void check_stuck_players()
    {
        auto max = game::Dvar_FindVar("sv_maxclients")->current.integer;
        for (int i = 0; i < max; i++)
        {
            auto* client = &game::svs_clients[i];
            if (client->state < game::CS_ACTIVE && client->state != game::CS_FREE)
            {
                if (warn_list.contains(i))
                {
//                  Might assign the zombie header type to the player for some time before finally removing them
                    game::SV_DropClientInternal(client, "EXE_TRANSMITERROR", 1);
                    warn_list.erase(i);
                    return;
                }

//              If the player is still connecting after 60 seconds, they will be kicked
                warn_list.insert(i);
            }
            else
            {
//              If they finally connected the player number should be removed
                warn_list.erase(i);
            }
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scheduler::loop(check_stuck_players, scheduler::pipeline::server, 1min);
        }

        void pre_destroy() override
        {
            warn_list.clear();
        }
    };
}

REGISTER_COMPONENT(zombie::component)
