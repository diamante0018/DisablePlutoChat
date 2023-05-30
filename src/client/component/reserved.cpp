#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "scheduler.hpp"

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/named_mutex.hpp>
#include <utils/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace reserved
{
  namespace
  {
    const char* reserved_list_file = "reserved-users.json";

    std::string reason;
    std::size_t max_reserved_count; // Limit VIPs per server

    using reserved_list_container = std::unordered_set<std::string>;
    utils::concurrency::container<reserved_list_container> reserved_list;

    // Have only one instance of IW5 read/write the file
    std::unique_lock<utils::named_mutex> lock()
    {
      static utils::named_mutex mutex{"iw5-reserved-list-lock"};
      std::unique_lock lock{mutex};
      return lock;
    }

    std::filesystem::path get_file_path()
    {
      std::filesystem::path root = "admin";
      return root / reserved_list_file;
    }

    rapidjson::Document parse_reserved_list()
    {
      rapidjson::Document default_doc{};
      default_doc.SetObject();

      const auto _ = lock();

      if (!utils::io::file_exists(get_file_path().string()))
      {
        printf("%s list file does not exist\n", reserved_list_file);
        return default_doc;
      }

      auto data = utils::io::read_file(get_file_path().string());
      if (data.empty())
      {
        printf("%s is empty\n", reserved_list_file);
        return default_doc;
      }

#ifdef _DEBUG
      printf("Loading \"%s\" from \"%s\"\n",
             reserved_list_file,
             get_file_path().string().data());
#endif

      rapidjson::Document doc{};
      const rapidjson::ParseResult result = doc.Parse(data);

      if (!result || !doc.IsObject())
      {
        return default_doc;
      }

      return doc;
    }

    void load_reserved_list()
    {
      const auto list_document = parse_reserved_list();
      if (!list_document.HasMember("Reason"))
      {
        printf("%s does not contain JSON member \"Reason\". Defaulting to %s\n",
               reserved_list_file,
               reason.data());
      }
      else if (list_document["Reason"].IsString())
      {
        reason = list_document["Reason"].GetString();
      }

      if (!list_document.HasMember("Limit"))
      {
        printf("%s does not contain JSON member \"Limit\". Defaulting to %zu\n",
               reserved_list_file,
               max_reserved_count);
      }
      else if (list_document["Limit"].IsUint())
      {
        max_reserved_count = std::clamp<std::size_t>(
            list_document["Limit"].GetUint(), 1, game::MAX_CLIENTS);
      }

      if (!list_document.HasMember("GUID"))
      {
        printf(
            "%s does not contain JSON member \"GUID\". No user will be considered privileged\n",
            reserved_list_file);
        return;
      }

      const rapidjson::Value& member_list = list_document["GUID"];
      if (!member_list.IsArray() || member_list.Empty())
      {
        return;
      }

      reserved_list.access(
          [&member_list](reserved_list_container& list)
          {
            for (const auto& entry : member_list.GetArray())
            {
              if (entry.IsString())
              {
                list.insert(entry.GetString());
              }
            }
          });
    }

    void save_reserved_list()
    {
      const auto _ = lock();

      rapidjson::Document list_document;
      list_document.SetObject();

      list_document.AddMember("Reason", reason, list_document.GetAllocator());
      list_document.AddMember(
          "Limit", max_reserved_count, list_document.GetAllocator());

      rapidjson::Value guid_entries(rapidjson::kArrayType);
      auto copy_list = reserved_list.access<reserved_list_container>(
          [](reserved_list_container& list) -> reserved_list_container
          {
            return list;
          });

      assert(!copy_list.empty());

      for (const auto& entry : copy_list)
      {
        rapidjson::Value guid;
        guid.SetString(entry.data(), list_document.GetAllocator());

        guid_entries.PushBack(guid, list_document.GetAllocator());
      }

      list_document.AddMember(
          "GUID", guid_entries, list_document.GetAllocator());

      rapidjson::StringBuffer buffer{};
      rapidjson::Writer<rapidjson::StringBuffer,
                        rapidjson::Document::EncodingType,
                        rapidjson::ASCII<>>
          writer(buffer);
      list_document.Accept(writer);

      utils::io::write_file(
          get_file_path().string(),
          std::string{buffer.GetString(), buffer.GetLength()});

#ifdef _DEBUG
      printf("Saving \"%s\" to \"%s\"\n",
             reserved_list_file,
             get_file_path().string().data());
#endif

      printf(
          "Saved %zu users IDs to %s\n", copy_list.size(), reserved_list_file);
    }

    void reserve_client(const game::client_s* client)
    {
      reserved_list.access(
          [&client](reserved_list_container& list)
          {
            list.insert(client->playerGuid);
          });

      scheduler::once(save_reserved_list, scheduler::pipeline::async);

      printf("\"%s\" was given VIP\n", client->playerGuid);
      game::SV_GameSendServerCommand(
          client - game::svs_clients,
          game::SV_CMD_CAN_IGNORE,
          utils::string::va("%c \"You were given VIP\"", 0x65));
    }

    void reserve_client(const std::string& guid)
    {
      reserved_list.access(
          [&guid](reserved_list_container& list)
          {
            list.insert(guid);
          });

      printf("%s was given VIP\n", guid.data());
      scheduler::once(save_reserved_list, scheduler::pipeline::async);
    }

    game::client_s* get_player_by_num()
    {
      const command::params_sv params;

      auto* com_sv_running = game::Dvar_FindVar("sv_running");
      if (!com_sv_running->current.enabled)
      {
        return nullptr;
      }

      if (params.size() < 2)
      {
        printf("No player specified.\n");
        return nullptr;
      }

      const auto* s = params.get(1);

      for (auto i = 0; s[i]; ++i)
      {
        if (s[i] < '0' || s[i] > '9')
        {
          printf("Bad slot number: %s\n", s);
          return nullptr;
        }
      }

      auto idnum = std::atoi(s);
      if (idnum < 0 || idnum >= game::MAX_CLIENTS)
      {
        printf("Bad client slot: %i\n", idnum);
        return nullptr;
      }

      auto* client = &game::svs_clients[idnum];
      if (client->state == game::CS_FREE)
      {
        printf("Client %i is not active\n", idnum);
        return nullptr;
      }

      return client;
    }

    bool is_test_client(const game::client_s* client)
    {
      if (client->state == game::CS_FREE)
      {
        return false;
      }

      const auto* guid = client->playerGuid;
      return std::strstr(guid, "bot") != nullptr;
    }

    void add_reserved_commands()
    {
      command::add_sv(
          "reserveClient",
          [](const command::params_sv& params)
          {
            auto* com_sv_running = game::Dvar_FindVar("sv_running");
            if (!com_sv_running->current.enabled)
            {
              printf("Server is not running.\n");
              return;
            }

            const auto* cmd = params.get(0);
            if (params.size() < 2)
            {
              printf("Usage: %s <client number> : reserve this client\n", cmd);
              return;
            }

            if (utils::string::is_number(params.get(1)))
            {
              const auto* client = get_player_by_num();
              if (!client)
              {
                return;
              }

              if (is_test_client(client))
              {
                return;
              }

              reserve_client(client);

              return;
            }

            reserve_client(params.get(1));
          });
    }

    bool is_server_full()
    {
      auto i = 0;
      while (i < *game::svs_clientCount)
      {
        if (game::svs_clients[i].state == game::CS_FREE)
        {
          // Free slot was found
          break;
        }

        ++i;
      }

      return i == *game::svs_clientCount;
    }

    bool is_client_reserved(const char* guid)
    {
      assert(guid);
      if (!guid)
      {
        return false;
      }

      return reserved_list.access<bool>(
          [&guid](const reserved_list_container& list)
          {
            return list.contains(guid);
          });
    }

    std::size_t get_reserved_count()
    {
      std::size_t count = 0;
      for (auto i = 0; i < *game::svs_clientCount; ++i)
      {
        auto* client = &game::svs_clients[i];
        assert(client->state);

        const auto is_reserved = is_client_reserved(client->playerGuid);
        if (is_reserved)
        {
          ++count;
        }
      }

      return count;
    }

    const char* info_value_for_key_stub(const char* s, const char* key)
    {
      if (is_server_full())
      {
        // First kick test clients
        for (auto i = 0; i < *game::svs_clientCount; ++i)
        {
          auto* client = &game::svs_clients[i];
          assert(client->state);

          if (is_test_client(client))
          {
            game::SV_DropClientInternal(client, "EXE_DISCONNECTED", true);
            client->state = game::CS_FREE;
            return game::Info_ValueForKey(s, key);
          }
        }
      }

      // If the server is still full
      if (is_server_full())
      {
        // Limit this patch according to the settings
        if (get_reserved_count() >= max_reserved_count)
        {
          return game::Info_ValueForKey(s, key);
        }

        // Check if the user that is trying to connect is a VIP
        const auto* value = game::Info_ValueForKey(s, "xuid");
        auto is_reserved = is_client_reserved(value);
        if (is_reserved)
        {
          // Check if there are any non-VIPs
          // We can kick or if there are too many VIPs
          // If there are too many VIPs we do nothing

          for (auto i = 0; i < *game::svs_clientCount; ++i)
          {
            auto* client = &game::svs_clients[i];
            assert(client->state);

            is_reserved = is_client_reserved(client->playerGuid);

            // This is the first non-VIP client we found in a full server. That
            // means the client will get kicked. The count of VIPs already on
            // the server is also less than max_reserved_count
            if (!is_reserved)
            {
              game::SV_DropClientInternal(client, reason.data(), true);
              client->state = game::CS_FREE;
              break;
            }
          }
        }
      }

      return game::Info_ValueForKey(s, key);
    }
  } // namespace

  class component final : public component_interface
  {
   public:
    void post_unpack() override
    {
      reason = "EXE_DISCONNECTED"s;
      max_reserved_count = game::MAX_CLIENTS;
      utils::hook::call(0x572E60, info_value_for_key_stub);

      scheduler::once(add_reserved_commands, scheduler::pipeline::main);
    }
  };
} // namespace reserved

REGISTER_COMPONENT(reserved::component)
