#include <std_include.hpp>
#include <utils/nt.hpp>

namespace game
{
  namespace
  {
    const utils::nt::library& get_host_library()
    {
      static const auto host_library = []
      {
        utils::nt::library host{};
        if (!host || host == utils::nt::library::get_by_address(get_base))
        {
          throw std::runtime_error("Invalid host application");
        }

        return host;
      }();

      return host_library;
    }
  } // namespace

  std::size_t get_base()
  {
    static const auto base =
        reinterpret_cast<std::size_t>(get_host_library().get_ptr());
    return base;
  }

  bool is_plutonium()
  {
    static const auto plutonium =
        get_host_library().get_optional_header()->CheckSum == 0x4EEAD0;
    return plutonium;
  }
} // namespace game
