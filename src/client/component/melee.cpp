#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace melee
{
  game::dvar_t* g_allowMelee;
  void fire_melee_stub(game::gentity_s* ent, int game_time)
  {
    if (g_allowMelee->current.enabled)
    {
      utils::hook::invoke<void>(0x530DB0, ent, game_time);
    }
  }

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      g_allowMelee = game::Dvar_RegisterBool(
          "g_allowMelee", true, game::NONE, "Allow the usage of the knife");
      utils::hook::call(0x4F7AB9, fire_melee_stub);
    }
  };
} // namespace melee

REGISTER_COMPONENT(melee::component)
