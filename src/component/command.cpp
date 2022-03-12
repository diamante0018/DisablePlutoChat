#include <stdinc.hpp>

#include "loader/component_loader.hpp"
#include "utils/nt.hpp"
#include "utils/string.hpp"

#include "command.hpp"

constexpr auto CMD_MAX_NESTING = 8;

namespace command
{
  namespace
  {
    std::unordered_map<std::string, std::function<void(params&)>> handlers;

    void cmd_say_as_player_f(const params& params)
    {
      if (params.size() < 3)
      {
        return;
      }

      const auto playerNum = std::atoi(params.get(1));

      if (playerNum >= *game::svs_clientCount)
      {
        return;
      }

      const auto player = &game::g_entities[playerNum];

      if (player->client == nullptr) return;

      game::Cmd_Say_f(player, 0, 0, params.join(2).data());
    }
  } // namespace

  void main_handler()
  {
    params params = {};

    const auto command = utils::string::to_lower(params[0]);
    if (handlers.find(command) != handlers.end())
    {
      handlers[command](params);
    }
  }

  params::params()
      : nesting_(game::cmd_args->nesting)
  {
    assert(game::cmd_args->nesting < CMD_MAX_NESTING);
  }

  int params::size() const
  {
    return game::cmd_args->argc[this->nesting_];
  }

  const char* params::get(const int index) const
  {
    if (index >= this->size())
    {
      return "";
    }

    return game::cmd_args->argv[this->nesting_][index];
  }

  std::string params::join(const int index) const
  {
    std::string result = {};

    for (auto i = index; i < this->size(); i++)
    {
      if (i > index) result.append(" ");
      result.append(this->get(i));
    }
    return result;
  }

  void add_raw(const char* name, void (*callback)())
  {
    game::Cmd_AddCommandInternal(
        name,
        callback,
        utils::memory::get_allocator()->allocate<game::cmd_function_t>());
  }

  void add(const char* name, const std::function<void(const params&)>& callback)
  {
    const auto command = utils::string::to_lower(name);

    if (handlers.find(command) == handlers.end())
    {
      add_raw(name, main_handler);
    }

    handlers[command] = callback;
  }

  void add(const char* name, const std::function<void()>& callback)
  {
    add(name, [callback](const command::params&)
    {
      callback();
    });
  }

  std::vector<std::string> script_commands;
  utils::memory::allocator allocator;

  void add_script_command(const std::string& name,
                          const std::function<void(const params&)>& callback)
  {
    script_commands.push_back(name);
    const auto _name = allocator.duplicate_string(name);
    add(_name, callback);
  }

  void clear_script_commands()
  {
    for (const auto& name : script_commands)
    {
      handlers.erase(name);
      game::Cmd_RemoveCommand(name.data());
    }

    allocator.clear();
    script_commands.clear();
  }

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      add_commands_generic();
    }

    void pre_destroy() override
    {
      clear_script_commands();
    }

   private:
    static void add_commands_generic()
    {
      add("quitMeme", utils::nt::raise_hard_exception);
      add("sayAsPlayer", cmd_say_as_player_f);
    }
  };
} // namespace command

REGISTER_COMPONENT(command::component)
