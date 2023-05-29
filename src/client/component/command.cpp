#include <std_include.hpp>

#include <utils/nt.hpp>
#include <utils/string.hpp>

#include "command.hpp"

constexpr auto CMD_MAX_NESTING = 8;

namespace command
{
  namespace
  {
    std::unordered_map<std::string, std::function<void(const params&)>>
        handlers;
    std::unordered_map<std::string, std::function<void(const params_sv&)>>
        handlers_sv;
  } // namespace

  void main_handler()
  {
    params params;

    const auto command = utils::string::to_lower(params[0]);

    if (const auto itr = handlers.find(command); itr != handlers.end())
    {
      itr->second(params);
    }
  }

  void main_handler_sv()
  {
    params_sv params;

    const auto command = utils::string::to_lower(params[0]);

    if (const auto itr = handlers_sv.find(command); itr != handlers_sv.end())
    {
      itr->second(params);
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
    std::string result;

    for (auto i = index; i < this->size(); i++)
    {
      if (i > index)
      {
        result.push_back(' ');
      }

      result.append(this->get(i));
    }

    return result;
  }

  params_sv::params_sv()
      : nesting_(game::sv_cmd_args->nesting)
  {
    assert(game::sv_cmd_args->nesting < CMD_MAX_NESTING);
  }

  int params_sv::size() const
  {
    return game::sv_cmd_args->argc[this->nesting_];
  }

  const char* params_sv::get(const int index) const
  {
    if (index >= this->size())
    {
      return "";
    }

    return game::sv_cmd_args->argv[this->nesting_][index];
  }

  std::string params_sv::join(const int index) const
  {
    std::string result;

    for (auto i = index; i < this->size(); i++)
    {
      if (i > index)
      {
        result.push_back(' ');
      }

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

  void add_raw_sv(const char* name, void (*callback)())
  {
    game::Cmd_AddServerCommandInternal(
        name,
        callback,
        utils::memory::get_allocator()->allocate<game::cmd_function_t>());

    add_raw(name, game::Cbuf_AddServerText_f);
  }

  void add(const char* name, const std::function<void(const params&)>& callback)
  {
    const auto command = utils::string::to_lower(name);

    if (!handlers.contains(command))
    {
      add_raw(name, main_handler);
    }

    handlers[command] = callback;
  }

  void add_sv(const char* name,
              const std::function<void(const params_sv&)>& callback)
  {
    const auto command = utils::string::to_lower(name);

    if (!handlers_sv.contains(command))
    {
      add_raw_sv(name, main_handler_sv);
    }

    handlers_sv[command] = callback;
  }

  void add(const char* name, const std::function<void()>& callback)
  {
    add(name,
        [callback](const command::params&)
        {
          callback();
        });
  }
} // namespace command
