#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/named_mutex.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "command.hpp"

namespace chat
{
  namespace
  {
    const auto* file_path = "admin/mute.json";
    utils::named_mutex file_mutex{"mw3-mute-list-lock"};

    std::mutex access_mutex;
    using mute = std::unordered_set<std::string>;
    mute mute_list{};

    game::dvar_t* sv_enableGameChat;

    utils::hook::detour client_command_hook;

    void save_mute_list(const mute& list)
    {
      std::unique_lock _{file_mutex};

      rapidjson::Document mute_data;
      mute_data.SetObject();
      rapidjson::Value mute_entries(rapidjson::kArrayType);

      for (const auto& entry : list)
      {
        rapidjson::Value values;
        values.SetObject();

        rapidjson::Value id;
        id.SetString(entry.data(), mute_data.GetAllocator());

        values.AddMember("SteamID", id, mute_data.GetAllocator());

        mute_entries.PushBack(values, mute_data.GetAllocator());
      }

      mute_data.AddMember(
          "MutedPlayers", mute_entries, mute_data.GetAllocator());

      rapidjson::StringBuffer buffer{};
      rapidjson::Writer<rapidjson::StringBuffer,
                        rapidjson::Document::EncodingType,
                        rapidjson::ASCII<>>
          writer(buffer);
      mute_data.Accept(writer);

      utils::io::write_file(file_path, buffer.GetString());
    }

    void load_mute_list(mute& list)
    {
      std::unique_lock _{file_mutex};

      if (!utils::io::file_exists(file_path))
      {
        printf("Mute list file does not exist\n");
        return;
      }

      rapidjson::Document mute_data;
      const rapidjson::ParseResult result =
          mute_data.Parse(utils::io::read_file(file_path).data());

      if (!result || !mute_data.IsObject())
      {
        printf("Failed to parse mute list file. Empty?\n");
        return;
      }

      if (!mute_data.HasMember("MutedPlayers"))
      {
        printf("Mute list file seems to be empty\n");
        return;
      }

      const rapidjson::Value& loaded_mute_list = mute_data["MutedPlayers"];

      if (!loaded_mute_list.IsArray() || loaded_mute_list.Empty())
      {
        return;
      }

      for (const auto& entry : loaded_mute_list.GetArray())
      {
        if (!entry.HasMember("SteamID"))
        {
          continue;
        }

        const auto& steam_id = entry["SteamID"];
        if (steam_id.GetType() == rapidjson::Type::kStringType)
        {
          list.insert(steam_id.GetString());
        }
      }
    }

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
        const auto* guid = game::svs_clients[client_num].playerGuid;
        if (std::strstr(guid, "bot") != nullptr)
        {
          client_command_hook.invoke<void>(client_num);
          return;
        }

        std::unique_lock _(access_mutex);
        if (!sv_enableGameChat->current.enabled || mute_list.contains(guid))
        {
          game::Cbuf_InsertText(
              game::LOCAL_CLIENT_0,
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
      player_num = std::min<std::uint32_t>(player_num, game::MAX_CLIENTS - 1);
      const auto* ent = &game::g_entities[player_num];
      if (ent->client == nullptr)
      {
        return;
      }

      const auto* guid = game::svs_clients[player_num].playerGuid;
      if (std::strstr(guid, "bot") != nullptr)
      {
        return;
      }

      printf("Muting %d: %s\n", player_num, guid);

      std::unique_lock _(access_mutex);
      if (mute_list.contains(guid))
      {
        return;
      }

      mute_list.insert(guid);
      save_mute_list(mute_list);
    }

    void cmd_umute_player_f(const command::params& params)
    {
      if (params.size() < 2)
      {
        return;
      }

      auto player_num = std::strtoul(params.get(1), nullptr, 10);
      player_num = std::min<std::uint32_t>(player_num, game::MAX_CLIENTS - 1);
      const auto* ent = &game::g_entities[player_num];
      if (ent->client == nullptr)
      {
        return;
      }

      const auto* guid = game::svs_clients[player_num].playerGuid;
      if (std::strstr(guid, "bot") != nullptr)
      {
        return;
      }

      printf("Unmuting %d: %s\n", player_num, guid);

      std::unique_lock _(access_mutex);
      if (!mute_list.contains(guid))
      {
        return;
      }

      mute_list.erase(guid);
      save_mute_list(mute_list);
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

      load_mute_list(mute_list);
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
