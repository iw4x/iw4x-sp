#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "network.hpp"

#include <utils/hook.hpp>

namespace network {
class component final : public component_interface {
public:
  void post_load() override { patch_sp(); }

private:
  static void patch_sp() {
    // Ignore protocol mismatch
    utils::hook::set<std::uint8_t>(0x65D0C2, 0xEB);
    utils::hook::set<std::uint8_t>(0x65D0ED, 0xEB);

    // Disable matchmaking stuff
    utils::hook::set<std::uint8_t>(0x43BAE0, 0xEB);

    // Ignore 'MAX_PACKET_USERCMDS'
    utils::hook::set<std::uint8_t>(0x4B1436, 0xEB);

    // Disable IWNet stuff
    utils::hook::set<std::uint8_t>(0x44E824, 0xEB);
    utils::hook::set<std::uint8_t>(0x44E808, 0xEB);

    // Disable MP packet handler
    utils::hook::set<std::uint8_t>(0x65E717, 0xEB);

    // Disable LSP packet handler
    utils::hook::set<std::uint8_t>(0x65E3A4, 0xEB);

    // Avoid spam
    utils::hook(0x65E786, game::Com_DPrintf, HOOK_CALL).install()->quick();
    utils::hook(0x65D659, game::Com_DPrintf, HOOK_CALL).install()->quick();

    // Disable BigShort in NET_AdrToString
    utils::hook(0x4BF4D1, game::ShortNoSwap, HOOK_CALL).install()->quick();
    utils::hook(0x4BF50C, game::ShortNoSwap, HOOK_CALL).install()->quick();

    // Disable BigShort in NET_StringToAdr
    utils::hook(0x40A657, game::ShortNoSwap, HOOK_CALL).install()->quick();
    utils::hook(0x40A676, game::ShortNoSwap, HOOK_CALL).install()->quick();

    // Disable BigShort in GetLocalAddressForEncryptedConnection
    utils::hook(0x4CF42D, game::ShortNoSwap, HOOK_CALL).install()->quick();

    // Parse port as unsigned short in Net_AddrToString
    utils::hook::set<const char*>(0x4BF4F3, "%u.%u.%u.%u:%hu");

    // Force Win socket initialization
    utils::hook::nop(0x42B649, 2);

    // Kill LSP
    utils::hook::set<std::uint8_t>(0x4553F0, 0xC3); // Hello
    utils::hook::set<std::uint8_t>(0x428D00, 0xC3); // LSP_SendLogRequest
    utils::hook::set<std::uint8_t>(0x4D13C0, 0xC3); // LSP_ParsePacket
    utils::hook::set<std::uint8_t>(0x66D440, 0xC3); // LSP_AddKeepAlive
    utils::hook::nop(0x494E68, 5);                  // Don't create LSP socket
  }
};
} // namespace network

REGISTER_COMPONENT(network::component)
