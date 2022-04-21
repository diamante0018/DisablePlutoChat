#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

constexpr auto DEFAULT_VIEWHEIGHT = 60;
constexpr auto PRONE_VIEWHEIGHT = 11;
constexpr auto CROUCH_VIEWHEIGHT = 40;
constexpr auto DEAD_VIEWHEIGHT = 8;

namespace sprint
{

  bool check_target_view_height(game::playerState_s* ps)
  {
    if (ps->viewHeightCurrent <= PRONE_VIEWHEIGHT ||
        ps->viewHeightCurrent >= CROUCH_VIEWHEIGHT)
    {
      return true;
    }

    ps->pm_flags &= ~(game::PMF_WALKING | game::PMF_DUCKED);

    ps->pm_flags |= game::PMF_PRONE;
    ps->eFlags |= game::EF_PRONE;

    ps->viewHeightCurrent = PRONE_VIEWHEIGHT;
    ps->viewHeightTarget = PRONE_VIEWHEIGHT;

    return false;
  }

  // clang-format off
  __declspec(naked) void pm_start_sprint_stub()
  {
    __asm
    {
      pushad;

      push esi; // ps
      call check_target_view_height;
      add esp, 4;

      test al, al;

      popad;

      jz skip; // If false was returned skip PM_StartSprint

      test ah, 0x41; // Code hook skipped
      jnz skip;

      push 0x41D67A; // Inlined 'call' to PM_StartSprint
      ret;

    skip:
      push 0x41D6A5;
      ret;
    }
  }
  // clang-format on

  class sprint final : public component_interface
  {
   public:
    void post_unpack() override
    {
      // Prevent player from sprinting when between prone and ducked
      utils::hook::jump(0x41D675, pm_start_sprint_stub);
    }
  };
} // namespace sprint

REGISTER_COMPONENT(sprint::sprint)
