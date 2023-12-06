#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "raw_file.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/flags.hpp>

#include <zlib.h>

namespace assets {
namespace {
utils::hook::detour db_read_raw_file_hook;
utils::hook::detour com_load_info_string_hook;

char* db_read_raw_file_stub(const char* filename, char* buf, int size) {
  auto file_handle = 0;
  const auto file_size = game::FS_FOpenFileRead(filename, &file_handle);

  if (file_handle) {
    if ((file_size + 1) <= size) {
      game::FS_Read(buf, file_size, file_handle);
      buf[file_size] = '\0';
      game::FS_FCloseFile(file_handle);
      return buf;
    }

    game::FS_FCloseFile(file_handle);
    game::Com_PrintError(
        game::CON_CHANNEL_ERROR,
        "Ignoring raw file '%s' as it exceeds buffer size %i > %i\n", filename,
        file_size, size);
  }

  auto* rawfile =
      game::DB_FindXAssetHeader(game::ASSET_TYPE_RAWFILE, filename).rawfile;
  if (game::DB_IsXAssetDefault(game::ASSET_TYPE_RAWFILE, filename)) {
    return nullptr;
  }

  game::DB_GetRawBuffer(rawfile, buf, size);
  return buf;
}

const char* com_load_info_string_fast_file(const char* file_name,
                                           const char* file_desc,
                                           const char* ident,
                                           char* load_buffer) {
  const static DWORD Com_LoadInfoString_FastFile_t = 0x602FA0;
  const char* result{};

  __asm {
    pushad;

    mov ebx, load_buffer;
    mov edi, file_name;
    push ident;
    push file_desc;
    call Com_LoadInfoString_FastFile_t;
    add esp, 0x8;
    mov result, eax;

    popad;
  }

  return result;
}

const char* com_load_info_string_load_obj(const char* file_name,
                                          const char* file_desc,
                                          const char* ident,
                                          char* load_buffer) {
  int file_handle;

  const auto file_len =
      game::FS_FOpenFileByMode(file_name, &file_handle, game::FS_READ);
  if (file_len < 0) {
    game::Com_DPrintf(game::CON_CHANNEL_SYSTEM,
                      "Could not load %s [%s] as rawfile\n", file_desc,
                      file_name);
    return nullptr;
  }

  const auto ident_len = static_cast<int>(std::strlen(ident));
  game::FS_Read(load_buffer, ident_len, file_handle);
  load_buffer[ident_len] = '\0';

  if (std::strncmp(load_buffer, ident, ident_len) != 0) {
    game::Com_Error(game::ERR_DROP,
                    "\x15"
                    "File [%s] is not a %s\n",
                    file_name, file_desc);
    return nullptr;
  }

  if ((file_len - ident_len) >= 0x4000) {
    game::Com_Error(game::ERR_DROP,
                    "\x15"
                    "File [%s] is too long of a %s to parse\n",
                    file_name, file_desc);
    return nullptr;
  }

  game::FS_Read(load_buffer, file_len - ident_len, file_handle);
  load_buffer[file_len - ident_len] = '\0';
  game::FS_FCloseFile(file_handle);

  return load_buffer;
}

const char* com_load_info_string_stub(const char* file_name,
                                      const char* file_desc, const char* ident,
                                      char* load_buffer) {
  const auto* buffer =
      com_load_info_string_load_obj(file_name, file_desc, ident, load_buffer);
  if (!buffer) {
    buffer = com_load_info_string_fast_file(file_name, file_desc, ident,
                                            load_buffer);
  }

  if (!game::Info_Validate(buffer)) {
    game::Com_Error(game::ERR_DROP,
                    "\x15"
                    "File [%s] is not a valid %s\n",
                    file_name, file_desc);
    return nullptr;
  }

  return buffer;
}

bool is_enabled() { IS_FLAG_ENABLED(dump_raw_file); }
} // namespace

void process_raw_file(game::XAssetHeader* header) {
  if (!is_enabled()) {
    return;
  }

  const auto* raw_file = header->rawfile;
  const auto filename = utils::string::va("raw/{0}", raw_file->name);

  if (raw_file->compressedLen > 0) {
    std::vector<std::uint8_t> uncompressed;
    uncompressed.resize(raw_file->len);

    if (uncompress(uncompressed.data(), (uLongf*)&raw_file->len,
                   reinterpret_cast<const Bytef*>(raw_file->buffer),
                   raw_file->compressedLen) == Z_OK) {
      std::string data;
      data.assign(uncompressed.begin(), uncompressed.end());

      utils::io::write_file(filename, data);
    }

    return;
  }

  // If uncompressed just dump it
  utils::io::write_file(filename, raw_file->buffer);
}

class raw_file final : public component_interface {
public:
  void post_load() override {
    // Remove fs_game check for moddable raw files
    // allows non-fs_game to modify raw files
    utils::hook::nop(0x612932, 2);

    db_read_raw_file_hook.create(0x46DA60, &db_read_raw_file_stub);

    com_load_info_string_hook.create(0x42DB20, &com_load_info_string_stub);
  }

  void pre_destroy() override { db_read_raw_file_hook.clear(); }
};
} // namespace assets

REGISTER_COMPONENT(assets::raw_file)
