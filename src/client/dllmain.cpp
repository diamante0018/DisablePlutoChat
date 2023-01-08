#include "std_include.hpp"
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "component/signatures.hpp"

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call,
                      LPVOID /*lpReserved*/
    )
{
  if (ul_reason_for_call == DLL_PROCESS_ATTACH)
  {
    if (!signatures::process())
    {
      MessageBoxA(
          nullptr,
          "This version of DisablePlutoChat is outdated.\n"
          "Download the latest dll from here: https://github.com/diamante0018/DisablePlutoChat/releases",
          "ERROR",
          MB_ICONERROR);

      return FALSE;
    }

    if (game::plutonium::printf.get() != nullptr)
    {
      utils::hook::jump(reinterpret_cast<uintptr_t>(&printf),
                        game::plutonium::printf);
    }

    component_loader::post_unpack();
  }

  else if (ul_reason_for_call == DLL_PROCESS_DETACH)
  {
    component_loader::pre_destroy();
  }

  return TRUE;
}
