#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

constexpr auto DEFAULT_VIEWHEIGHT = 60;
constexpr auto PRONE_VIEWHEIGHT = 11;
constexpr auto CROUCH_VIEWHEIGHT = 40;
constexpr auto DEAD_VIEWHEIGHT = 8;

namespace sprint
{
  namespace
  {
    bool check_target_view_height(const game::playerState_s* ps)
    {
      return (ps->viewHeightCurrent <= PRONE_VIEWHEIGHT ||
              ps->viewHeightCurrent >= CROUCH_VIEWHEIGHT);
    }

    __declspec(naked) void pm_start_sprint_stub()
    {
      __asm {
        pushad

        push esi // ps
        call check_target_view_height
        add esp, 4

        test al, al

        popad

                // If false was returned skip PM_StartSprint
        jz skip

                    // Game code hook skipped
        test ah, 0x41
        jnz skip

            // Inlined 'call' to PM_StartSprint
        push 0x41D67A
        retn

      skip:
        push 0x41D6A5
        retn
      }
    }
  } // namespace
  class sprint final : public component_interface
  {
   public:
    void post_unpack() override
    {
      utils::hook::jump(0x41D675, pm_start_sprint_stub);
    }
  };
} // namespace sprint

REGISTER_COMPONENT(sprint::sprint)
