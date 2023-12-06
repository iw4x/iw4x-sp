#include <std_include.hpp>
#include <utils/io.hpp>
#include <utils/cryptography.hpp>

#include "binary_loader.hpp"

#define SP_HASH                                                                \
  "39E9C4FACEA4B19017BB8C2FC64E4149708927AC40C7C3DEACC25BDD25E93D32"

#define SP_XLABS_HASH                                                          \
  "05D499D77028859D4BA30C852DA85CCA5F02678B22AEA9E27D7C56973B14A0BC"

namespace binary_loader {
std::string load_base() {
  std::string data;
  if (!utils::io::read_file("iw4sp.exe", &data)) {
    throw std::runtime_error("Failed to read game binary (iw4sp.exe)!");
  }

  const auto hash = utils::cryptography::sha256::compute(data, true);
  if ((hash != SP_XLABS_HASH) && (hash != SP_HASH)) {
    throw std::runtime_error(
        "Your iw4sp.exe is incompatible with this client.");
  }

  return data;
}

std::string load() { return load_base(); }
} // namespace binary_loader
