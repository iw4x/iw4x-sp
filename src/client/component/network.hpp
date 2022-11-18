#pragma once

namespace network {
using callback = std::function<void(const game::netadr_t&, const std::string&)>;
}
