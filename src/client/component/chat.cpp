#include <std_include.hpp>
#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "command.hpp"

namespace chat
{
  namespace
  {
    std::mutex access_mutex;
    std::unordered_set<std::int32_t> mute_list{};

    game::dvar_t* sv_enableGameChat;

    utils::hook::detour client_command_hook;

    void client_command_stub(int client_num)
    {
      char cmd[1024]{};
      const auto* ent = &game::g_entities[client_num];

      if (ent->client == nullptr)
      {
        // Not fully in game yet
        return;
      }

      game::SV_Cmd_ArgvBuffer(0, cmd, sizeof(cmd));

      if (utils::string::starts_with(cmd, "say"))
      {
        std::unique_lock _(access_mutex);
        if (!sv_enableGameChat->current.enabled ||
            mute_list.contains(client_num))
        {
          game::Cbuf_InsertText(
              0,
              utils::string::va(
                  "tell %d \"You are not allowed to type in the chat\"",
                  client_num));
          return;
        }
      }

      client_command_hook.invoke<void>(client_num);
    }

    void cmd_mute_player_f(const command::params& params)
    {
      if (params.size() < 2)
      {
        return;
      }

      auto player_num = std::strtoul(params.get(1), nullptr, 10);
      player_num = std::min<std::uint32_t>(player_num, 18);

      std::unique_lock _(access_mutex);
      if (mute_list.contains(player_num))
      {
        return;
      }

      mute_list.insert(player_num);
    }

    void cmd_umute_player_f(const command::params& params)
    {
      if (params.size() < 2)
      {
        return;
      }

      auto player_num = std::strtoul(params.get(1), nullptr, 10);
      player_num = std::min<std::uint32_t>(player_num, 18);

      std::unique_lock _(access_mutex);
      if (!mute_list.contains(player_num))
      {
        return;
      }

      mute_list.erase(player_num);
    }
  } // namespace

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      client_command_hook.create(0x502CB0, &client_command_stub);

      add_chat_commands();

      sv_enableGameChat = game::Dvar_RegisterBool(
          "sv_enableGameChat", true, game::NONE, "Enable game chat");
    }

   private:
    static void add_chat_commands()
    {
      command::add("mutePlayer", cmd_mute_player_f);
      command::add("unmutePlayer", cmd_umute_player_f);
    }
  };
} // namespace chat

REGISTER_COMPONENT(chat::component)
