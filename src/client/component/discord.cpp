#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <discord_rpc.h>

#include "scheduler.hpp"

using namespace std::literals;

namespace discord {
namespace {
DiscordRichPresence discord_presence;

void update_discord() {
  Discord_RunCallbacks();

  if (!discord_presence.startTimestamp) {
    discord_presence.startTimestamp =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
  }

  Discord_UpdatePresence(&discord_presence);
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    DiscordEventHandlers handlers;
    ZeroMemory(&handlers, sizeof(handlers));
    handlers.ready = ready;
    handlers.errored = errored;
    handlers.disconnected = errored;
    handlers.joinGame = nullptr;
    handlers.spectateGame = nullptr;
    handlers.joinRequest = nullptr;

    Discord_Initialize("978049907585863710", &handlers, 1, nullptr);

    scheduler::once(
        [] {
          scheduler::once(update_discord, scheduler::pipeline::async);
          scheduler::loop(update_discord, scheduler::pipeline::async, 15s);
        },
        scheduler::pipeline::main);

    initialized_ = true;
  }

  void pre_destroy() override {
    if (initialized_) {
      Discord_Shutdown();
    }
  }

private:
  bool initialized_ = false;

  static void ready(const DiscordUser* request) {
    ZeroMemory(&discord_presence, sizeof(discord_presence));

    discord_presence.state = "Singleplayer";
    discord_presence.instance = 1;
    discord_presence.startTimestamp = 0;
    printf("Discord: Ready\n");
    Discord_UpdatePresence(&discord_presence);
  }

  static void errored(const int error_code, const char* message) {
    printf("Discord: (%i) %s", error_code, message);
  }
};
} // namespace discord

REGISTER_COMPONENT(discord::component)
