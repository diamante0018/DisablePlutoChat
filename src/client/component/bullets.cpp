#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace bullets
{
  utils::hook::detour bullet_should_penetrate_hook;
  utils::hook::detour bullet_should_spread_hook;

  bool bullet_should_penetrate_stub(const unsigned int* const, game::Weapon,
                                    bool)
  {
    return true;
  }

  bool bullet_should_spread_stub(const unsigned int* const, game::Weapon, bool)
  {
    return false;
  }

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      bullet_should_penetrate_hook.create(0x431250,
                                          &bullet_should_penetrate_stub);
      bullet_should_spread_hook.create(0x438F00, &bullet_should_spread_stub);
    }

    void pre_destroy() override
    {
      bullet_should_penetrate_hook.clear();
      bullet_should_spread_hook.clear();
    }
  };
} // namespace bullets

REGISTER_COMPONENT(bullets::component)
