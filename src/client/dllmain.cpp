#include "std_include.hpp"
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call,
                      LPVOID /*lpReserved*/
)
{
  if (ul_reason_for_call == DLL_PROCESS_ATTACH)
  {
    if (!game::is_plutonium())
    {
      MessageBoxA(nullptr, "This plugin is outdated.", "ERROR", MB_ICONERROR);

      return FALSE;
    }

    component_loader::post_unpack();
  }

  else if (ul_reason_for_call == DLL_PROCESS_DETACH)
  {
    component_loader::pre_destroy();
  }

  return TRUE;
}
