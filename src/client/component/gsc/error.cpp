#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace gsc {
namespace hunk {
game::HunkUser* g_debug_user_;

void user_destroy(game::HunkUser* user) {
  auto* current = user->next;
  while (current) {
    auto* next = current->next;
    ::VirtualFree(current, 0, MEM_RELEASE);
    current = next;
  }

  ::VirtualFree(user, 0, MEM_RELEASE);
}

void init_debug_memory() {
  assert(game::Sys_IsMainThread());
  assert(!g_debug_user_);

  g_debug_user_ =
      game::Hunk_UserCreate(0x1000000, "Hunk_InitDebugMemory", false, 0);
}

void shutdown_debug_memory() {
  assert(game::Sys_IsMainThread());
  assert(g_debug_user_);

  user_destroy(g_debug_user_);
  g_debug_user_ = nullptr;
}

void* alloc_debug_mem(int size) {
  assert(game::Sys_IsMainThread());
  assert(g_debug_user_);

  return game::Hunk_UserAlloc(g_debug_user_, size, 4);
}

void free_debug_mem([[maybe_unused]] void* ptr) {
  assert(game::Sys_IsMainThread());
  assert(g_debug_user_);

  // Let's hope it gets cleared by Hunk_ShutdownDebugMemory
}
} // namespace hunk

namespace scr {
int developer_;

game::scrParserGlob_t parser_glob_;
game::scrParserPub_t parser_pub_;

int get_cumul_offset() {
  assert(game::scrCompileGlob->cumulOffset >= 0);
  return game::scrCompileGlob->cumulOffset;
}

int get_line_num_internal(const char* buf, unsigned int source_pos,
                          const char** start_line, int* col) {
  assert(buf);

  *start_line = buf;
  unsigned int line_num = 0;

  while (source_pos) {
    if (!*buf) {
      *start_line = buf + 1;
      ++line_num;
    }

    ++buf;
    --source_pos;
  }

  *col = buf - *start_line;
  return static_cast<int>(line_num);
}

int get_line_num(const unsigned int buffer_index,
                 const unsigned int source_pos) {
  const char* start_line;
  int col;

  assert(developer_);

  return get_line_num_internal(
      parser_pub_.sourceBufferLookup[buffer_index].sourceBuf, source_pos,
      &start_line, &col);
}

game::OpcodeLookup* get_prev_source_pos_opcode_lookup(const char* code_pos) {
  assert(game::Scr_IsInOpcodeMemory(code_pos));
  assert(parser_glob_.opcodeLookup);

  unsigned int low = 0;
  unsigned int high = parser_glob_.opcodeLookupLen - 1;

  while (low <= high) {
    const auto middle = (high + low) >> 1;

    if (code_pos < parser_glob_.opcodeLookup[middle].codePos) {
      high = middle - 1;
    } else {
      low = middle + 1;
      if (low == parser_glob_.opcodeLookupLen ||
          code_pos < parser_glob_.opcodeLookup[low].codePos) {
        return &parser_glob_.opcodeLookup[middle];
      }
    }
  }

  return nullptr;
}

unsigned int get_prev_source_pos(const char* code_pos,
                                 const unsigned int index) {
  return parser_glob_
      .sourcePosLookup[index + get_prev_source_pos_opcode_lookup(code_pos)
                                   ->sourcePosIndex]
      .sourcePos;
}

void copy_formatted_line(char* line, const char* raw_line) {
  auto len = std::strlen(raw_line);
  if (len >= 1024) {
    len = 1024 - 1;
  }

  for (std::size_t i = 0; i < len; ++i) {
    if (raw_line[i] == '\t') {
      line[i] = ' ';
    } else {
      line[i] = raw_line[i];
    }
  }

  if (line[len - 1] == '\r') {
    line[len - 1] = '\0';
  }

  line[len] = '\0';
}

unsigned int get_source_buffer(const char* code_pos) {
  unsigned int buffer_index;

  assert(game::Scr_IsInOpcodeMemory(code_pos));
  assert(parser_pub_.sourceBufferLookupLen > 0);

  for (buffer_index = parser_pub_.sourceBufferLookupLen - 1; buffer_index;
       --buffer_index) {
    if (!parser_pub_.sourceBufferLookup[buffer_index].codePos) {
      continue;
    }

    if (parser_pub_.sourceBufferLookup[buffer_index].codePos > code_pos) {
      continue;
    }

    break;
  }

  return buffer_index;
}

int get_line_info(const char* buf, const unsigned int source_pos, int* col,
                  char* line) {
  const char* start_line;
  unsigned int line_num;

  if (buf) {
    line_num = get_line_num_internal(buf, source_pos, &start_line, col);
  } else {
    line_num = 0;
    start_line = "";
    *col = 0;
  }

  copy_formatted_line(line, start_line);
  return static_cast<int>(line_num);
}

void print_source_pos(const int channel, const char* filename, const char* buf,
                      const unsigned int source_pos) {
  char line[1024]{};
  int col;

  assert(filename);
  const auto line_num = get_line_info(buf, source_pos, &col, line);

  game::Com_PrintMessage(
      channel,
      game::va("(file '%s'%s, line %d)\n", filename,
               parser_glob_.saveSourceBufferLookup ? " (savegame)" : "",
               line_num + 1),
      0);
  game::Com_PrintMessage(channel, game::va("%s\n", line), 0);

  for (auto i = 0; i < col; ++i) {
    game::Com_PrintMessage(channel, " ", 0);
  }

  game::Com_PrintMessage(channel, "*\n", 0);
}

void print_prev_code_pos(const int channel, const char* code_pos,
                         const unsigned int index) {
  if (!code_pos) {
    game::Com_PrintMessage(channel, "<frozen thread>\n", 0);
    return;
  }

  if (code_pos == game::g_EndPos) {
    game::Com_PrintMessage(channel, "<removed thread>\n", 0);
    return;
  }

  if (!developer_) {
    if (game::Scr_IsInOpcodeMemory(code_pos - 1)) {
      game::Com_PrintMessage(
          channel,
          game::va("@ %d\n", code_pos - game::scrVarPub->programBuffer), 0);
      return;
    }
  } else {
    if (game::scrVarPub->programBuffer &&
        game::Scr_IsInOpcodeMemory(code_pos)) {
      const auto buffer_index = get_source_buffer(code_pos - 1);

      print_source_pos(channel,
                       parser_pub_.sourceBufferLookup[buffer_index].buf,
                       parser_pub_.sourceBufferLookup[buffer_index].sourceBuf,
                       get_prev_source_pos(code_pos - 1, index));
      return;
    }
  }

  game::Com_PrintMessage(channel, game::va("%s\n\n", code_pos), 0);
}

void get_text_source_pos([[maybe_unused]] const char* buf, const char* code_pos,
                         char* line) {
  int col;

  if (developer_ && code_pos && code_pos != game::g_EndPos &&
      game::scrVarPub->programBuffer && game::Scr_IsInOpcodeMemory(code_pos)) {
    const auto buffer_index = get_source_buffer(code_pos - 1);

    get_line_info(parser_pub_.sourceBufferLookup[buffer_index].sourceBuf,
                  get_prev_source_pos(code_pos - 1, 0), &col, line);
  } else {
    *line = '\0';
  }
}

void init_opcode_lookup() {
  assert(!parser_glob_.opcodeLookup);
  assert(!parser_glob_.sourcePosLookup);
  assert(!parser_pub_.sourceBufferLookup);
  assert(!parser_pub_.codeOffsetMap);
  assert(!parser_pub_.useCodeOffsetMap);

  if (!developer_) {
    return;
  }

  parser_glob_.delayedSourceIndex = -1;
  parser_glob_.opcodeLookupMaxSize = 0;
  parser_glob_.opcodeLookupLen = 0;
  parser_glob_.opcodeLookup = static_cast<game::OpcodeLookup*>(::VirtualAlloc(
      nullptr, game::MAX_OPCODE_LOOKUP_SIZE, MEM_RESERVE, PAGE_READWRITE));

  parser_glob_.sourcePosLookupMaxSize = 0;
  parser_glob_.sourcePosLookupLen = 0;
  parser_glob_.sourcePosLookup = static_cast<game::SourceLookup*>(
      ::VirtualAlloc(nullptr, game::MAX_SOURCEPOS_LOOKUP_SIZE, MEM_RESERVE,
                     PAGE_READWRITE));

  parser_glob_.currentCodePos = nullptr;
  parser_glob_.currentSourcePosCount = 0;

  parser_glob_.sourceBufferLookupMaxSize = 0;
  parser_pub_.sourceBufferLookupLen = 0;
  parser_pub_.sourceBufferLookup = static_cast<game::SourceBufferInfo*>(
      ::VirtualAlloc(nullptr, game::MAX_SOURCEBUF_LOOKUP_SIZE, MEM_RESERVE,
                     PAGE_READWRITE));
}

void shutdown_opcode_lookup() {
  if (parser_glob_.opcodeLookup) {
    ::VirtualFree(parser_glob_.opcodeLookup, 0, MEM_RELEASE);
    parser_glob_.opcodeLookup = nullptr;
  }

  if (parser_glob_.sourcePosLookup) {
    ::VirtualFree(parser_glob_.sourcePosLookup, 0, MEM_RELEASE);
    parser_glob_.sourcePosLookup = nullptr;
  }

  if (parser_pub_.sourceBufferLookup) {
    for (unsigned int i = 0; i < parser_pub_.sourceBufferLookupLen; ++i) {
      hunk::free_debug_mem(parser_pub_.sourceBufferLookup[i].buf);
    }

    ::VirtualFree(parser_pub_.sourceBufferLookup, 0, MEM_RELEASE);
    parser_pub_.sourceBufferLookup = nullptr;
  }

  if (parser_glob_.saveSourceBufferLookup) {
    for (unsigned int i = 0; i < parser_glob_.saveSourceBufferLookupLen; ++i) {
      if (parser_glob_.saveSourceBufferLookup[i].sourceBuf) {
        hunk::free_debug_mem(parser_glob_.saveSourceBufferLookup[i].buf);
      }
    }

    hunk::free_debug_mem(parser_glob_.saveSourceBufferLookup);
    parser_glob_.saveSourceBufferLookup = nullptr;
  }
}

game::SourceBufferInfo* get_new_source_buffer() {
  assert(parser_pub_.sourceBufferLookup);

  auto size =
      sizeof(game::SourceBufferInfo) * (parser_pub_.sourceBufferLookupLen + 1);
  if (size > parser_glob_.sourceBufferLookupMaxSize) {
    if (parser_glob_.sourceBufferLookupMaxSize >=
        game::MAX_SOURCEBUF_LOOKUP_SIZE) {
      game::Sys_Error("MAX_SOURCEBUF_LOOKUP_SIZE exceeded");
    }

    ::VirtualAlloc((char*)parser_pub_.sourceBufferLookup +
                       parser_glob_.sourceBufferLookupMaxSize,
                   0x20000, MEM_COMMIT, PAGE_READWRITE);
    parser_glob_.sourceBufferLookupMaxSize += 0x20000;

    assert(size <= parser_glob_.sourceBufferLookupMaxSize);
  }

  return &parser_pub_.sourceBufferLookup[parser_pub_.sourceBufferLookupLen++];
}

void add_source_buffer_internal(const char* ext_filename, const char* code_pos,
                                char* source_buf, int len, bool do_eol_fixup,
                                bool archive) {
  int i;

  if (!parser_pub_.sourceBufferLookup) {
    parser_pub_.sourceBuf = nullptr;
    return;
  }

  assert((len >= -1));
  assert((len >= 0) || !source_buf);

  const auto str_len = std::strlen(ext_filename) + 1;
  const auto new_len = str_len + len + 2;
  auto* buf = static_cast<char*>(hunk::alloc_debug_mem(
      static_cast<int>(new_len))); // Scr_AddSourceBufferInternal

  strncpy_s(buf, new_len, ext_filename, _TRUNCATE);
  auto* source_buf2 = source_buf ? buf + str_len : nullptr;
  auto* source = source_buf;
  auto* dest = source_buf2;

  if (do_eol_fixup) {
    for (i = 0; i <= len; ++i) {
      const auto c = *source++;
      if (c == '\n' || c == '\r' && *source != '\n') {
        *dest = 0;
      } else {
        *dest = c;
      }

      ++dest;
    }
  } else {
    for (i = 0; i <= len; ++i) {
      const auto c = *source;
      ++source;
      *dest = c;
      ++dest;
    }
  }

  auto* buffer_info = get_new_source_buffer();
  buffer_info->codePos = code_pos;
  buffer_info->buf = buf;
  buffer_info->sourceBuf = source_buf2;
  buffer_info->len = len;
  buffer_info->sortedIndex = -1;
  buffer_info->archive = archive;

  if (source_buf2) {
    parser_pub_.sourceBuf = source_buf2;
  }
}

char* read_file_fast_file([[maybe_unused]] const char* filename,
                          const char* ext_filename, const char* code_pos,
                          [[maybe_unused]] const bool archive) {
  auto* rawfile =
      game::DB_FindXAssetHeader(game::ASSET_TYPE_RAWFILE, ext_filename).rawfile;
  if (game::DB_IsXAssetDefault(game::ASSET_TYPE_RAWFILE, ext_filename)) {
    add_source_buffer_internal(ext_filename, code_pos, nullptr, -1, true, true);
    return nullptr;
  }

  const auto len = game::DB_GetRawFileLen(rawfile);
  auto* source_buf = static_cast<char*>(
      game::Hunk_AllocateTempMemoryHigh(len)); // Scr_ReadFile_FastFile
  game::DB_GetRawBuffer(rawfile, source_buf, len);
  add_source_buffer_internal(ext_filename, code_pos, source_buf, len - 1, true,
                             true);
  return source_buf;
}

char* read_file_load_obj([[maybe_unused]] const char* filename,
                         const char* ext_filename, const char* code_pos,
                         const bool archive) {
  int f;

  auto len = game::FS_FOpenFileByMode(ext_filename, &f, game::FS_READ);
  if (len < 0) {
    add_source_buffer_internal(ext_filename, code_pos, nullptr, -1, true,
                               archive);
    return nullptr;
  }

  *game::g_loadedImpureScript = true;

  auto* source_buf =
      static_cast<char*>(game::Hunk_AllocateTempMemoryHigh(len + 1));
  game::FS_Read(source_buf, len, f);
  source_buf[len] = '\0';

  game::FS_FCloseFile(f);
  add_source_buffer_internal(ext_filename, code_pos, source_buf, len, true,
                             archive);

  return source_buf;
}

char* read_file(const char* filename, const char* ext_filename,
                const char* code_pos, const bool archive) {
  int file;

  if (game::FS_FOpenFileRead(ext_filename, &file) < 0) {
    return read_file_fast_file(filename, ext_filename, code_pos, archive);
  }

  game::FS_FCloseFile(file);
  return read_file_load_obj(filename, ext_filename, code_pos, archive);
}

char* add_source_buffer(const char* filename, const char* ext_filename,
                        const char* code_pos, const bool archive) {
  char* source_buf;

  if (archive && parser_glob_.saveSourceBufferLookup) {
    assert(parser_glob_.saveSourceBufferLookupLen > 0);

    --parser_glob_.saveSourceBufferLookupLen;

    const auto* save_source_buffer = parser_glob_.saveSourceBufferLookup +
                                     parser_glob_.saveSourceBufferLookupLen;
    const auto len = save_source_buffer->len;

    assert(len >= -1);

    if (len < 0) {
      add_source_buffer_internal(ext_filename, code_pos, nullptr, -1, true,
                                 archive);
      source_buf = nullptr;
    } else {
      source_buf =
          static_cast<char*>(game::Hunk_AllocateTempMemoryHigh(len + 1));

      const char* source = save_source_buffer->sourceBuf;
      auto* dest = source_buf;
      for (int i = 0; i < len; ++i) {
        const auto c = *source++;
        *dest = c ? c : '\n';
        dest++;
      }

      *dest = '\0';
      add_source_buffer_internal(ext_filename, code_pos, source_buf, len, false,
                                 archive);
    }

    return source_buf;
  }

  return read_file(filename, ext_filename, code_pos, archive);
}

unsigned int load_script_internal_stub(const char* filename,
                                       game::PrecacheEntry* entries,
                                       int entries_count) {
  char ext_filename[game::MAX_QPATH];
  game::sval_u parse_data;

  assert(game::scrCompilePub->script_loading);
  assert(std::strlen(filename) < game::MAX_QPATH);

  const auto name = game::Scr_CreateCanonicalFilename(filename);

  if (game::FindVariable(game::scrCompilePub->loadedscripts, name)) {
    game::SL_RemoveRefToString(name);
    const auto file_pos_ptr =
        game::FindVariable(game::scrCompilePub->scriptsPos, name);
    return file_pos_ptr
               ? game::FindObject(game::scrCompilePub->scriptsPos, file_pos_ptr)
               : 0;
  }

  const auto script_id =
      game::GetNewVariable(game::scrCompilePub->loadedscripts, name);
  game::SL_RemoveRefToString(name);

  sprintf_s(ext_filename, "%s.gsc",
            game::SL_ConvertToString(static_cast<unsigned short>(name)));

  const auto* old_source_buf = parser_pub_.sourceBuf;
  const auto* source_buffer = add_source_buffer(
      game::SL_ConvertToString(static_cast<unsigned short>(name)), ext_filename,
      game::TempMalloc(0), true);

  if (!source_buffer) {
    return 0;
  }

  const auto old_anim_tree_names = game::scrAnimPub->animTreeNames;
  game::scrAnimPub->animTreeNames = 0;
  game::scrCompilePub->far_function_count = 0;

  const auto* old_filename = parser_pub_.scriptfilename;
  parser_pub_.scriptfilename = ext_filename;

  game::scrCompilePub->in_ptr = "+";
  game::scrCompilePub->in_ptr_valid = false;
  game::scrCompilePub->parseBuf = source_buffer;

  game::ScriptParse(&parse_data, 0);
  game::scrCompilePub->parseBuf = nullptr;

  const auto file_pos_id =
      game::GetObject(game::scrCompilePub->scriptsPos,
                      game::GetVariable(game::scrCompilePub->scriptsPos, name));
  const auto file_count_id = game::GetObject(
      game::scrCompilePub->scriptsCount,
      game::GetVariable(game::scrCompilePub->scriptsCount, name));

  game::ScriptCompile(parse_data.node, file_pos_id, file_count_id, script_id,
                      entries, entries_count);

  game::RemoveVariable(game::scrCompilePub->scriptsCount, name);

  parser_pub_.scriptfilename = old_filename;
  parser_pub_.sourceBuf = old_source_buf;

  game::scrAnimPub->animTreeNames = old_anim_tree_names;

  return file_pos_id;
}

void settings_stub([[maybe_unused]] int developer, int developer_script,
                   int abort_on_error) {
  static_assert(offsetof(game::scrVarPub_t, developer_script) == 0x6);
  static_assert(offsetof(game::scrVmPub_t, abort_on_error) == 0x15);

  assert(!abort_on_error || developer);

  developer_ = (*dvars::com_developer)->current.enabled;
  game::scrVarPub->developer_script = developer_script != 0;
  game::scrVmPub->abort_on_error = abort_on_error != 0;
}
} // namespace scr

namespace {
void add_opcode_pos(const unsigned int source_pos, int type) {
  game::OpcodeLookup* opcode_lookup;
  game::SourceLookup* source_pos_lookup;

  if (!scr::developer_) {
    return;
  }

  if (game::scrCompilePub->developer_statement == 2) {
    assert(!game::scrVarPub->developer_script);
    return;
  }

  if (game::scrCompilePub->developer_statement == 3) {
    return;
  }

  if (!game::scrCompilePub->allowedBreakpoint) {
    type &= ~game::SOURCE_TYPE_BREAKPOINT;
  }

  assert(scr::parser_glob_.opcodeLookup);
  assert(scr::parser_glob_.sourcePosLookup);
  assert(game::scrCompilePub->opcodePos);

  auto size =
      sizeof(game::OpcodeLookup) * (scr::parser_glob_.opcodeLookupLen + 1);
  if (size > scr::parser_glob_.opcodeLookupMaxSize) {
    if (scr::parser_glob_.opcodeLookupMaxSize >= game::MAX_OPCODE_LOOKUP_SIZE) {
      game::Sys_Error("MAX_OPCODE_LOOKUP_SIZE exceeded");
    }

    ::VirtualAlloc((char*)scr::parser_glob_.opcodeLookup +
                       scr::parser_glob_.opcodeLookupMaxSize,
                   0x20000, MEM_COMMIT, PAGE_READWRITE);
    scr::parser_glob_.opcodeLookupMaxSize += 0x20000;

    assert(size <= scr::parser_glob_.opcodeLookupMaxSize);
  }

  size =
      sizeof(game::SourceLookup) * (scr::parser_glob_.sourcePosLookupLen + 1);
  if (size > scr::parser_glob_.sourcePosLookupMaxSize) {
    if (scr::parser_glob_.sourcePosLookupMaxSize >=
        game::MAX_SOURCEPOS_LOOKUP_SIZE) {
      game::Sys_Error("MAX_SOURCEPOS_LOOKUP_SIZE exceeded");
    }

    ::VirtualAlloc((char*)scr::parser_glob_.sourcePosLookup +
                       scr::parser_glob_.sourcePosLookupMaxSize,
                   0x20000, MEM_COMMIT, PAGE_READWRITE);
    scr::parser_glob_.sourcePosLookupMaxSize += 0x20000;

    assert(size <= scr::parser_glob_.sourcePosLookupMaxSize);
  }

  if (scr::parser_glob_.currentCodePos == game::scrCompilePub->opcodePos) {
    assert(scr::parser_glob_.currentSourcePosCount);

    --scr::parser_glob_.opcodeLookupLen;
    opcode_lookup =
        &scr::parser_glob_.opcodeLookup[scr::parser_glob_.opcodeLookupLen];

    assert(opcode_lookup->sourcePosIndex +
               scr::parser_glob_.currentSourcePosCount ==
           scr::parser_glob_.sourcePosLookupLen);
    assert(opcode_lookup->codePos == (char*)scr::parser_glob_.currentCodePos);
  } else {
    scr::parser_glob_.currentSourcePosCount = 0;
    scr::parser_glob_.currentCodePos = game::scrCompilePub->opcodePos;
    opcode_lookup =
        &scr::parser_glob_.opcodeLookup[scr::parser_glob_.opcodeLookupLen];
    opcode_lookup->sourcePosIndex = scr::parser_glob_.sourcePosLookupLen;
    opcode_lookup->codePos = scr::parser_glob_.currentCodePos;
    opcode_lookup->cumulOffset =
        static_cast<unsigned short>(scr::get_cumul_offset());
    // TODO: Assign a value to opcode_lookup->localVars
  }

  const auto source_pos_lookup_index =
      scr::parser_glob_.currentSourcePosCount + opcode_lookup->sourcePosIndex;
  source_pos_lookup =
      &scr::parser_glob_.sourcePosLookup[source_pos_lookup_index];
  source_pos_lookup->sourcePos = source_pos;

  if (source_pos == static_cast<unsigned int>(-1)) {
    assert(scr::parser_glob_.delayedSourceIndex == -1);
    assert(type & game::SOURCE_TYPE_BREAKPOINT);

    scr::parser_glob_.delayedSourceIndex =
        static_cast<int>(source_pos_lookup_index);
  } else if (source_pos == static_cast<unsigned int>(-2)) {
    scr::parser_glob_.threadStartSourceIndex =
        static_cast<int>(source_pos_lookup_index);
  } else if (scr::parser_glob_.delayedSourceIndex >= 0 &&
             (type & game::SOURCE_TYPE_BREAKPOINT)) {
    scr::parser_glob_.sourcePosLookup[scr::parser_glob_.delayedSourceIndex]
        .sourcePos = source_pos;
    scr::parser_glob_.delayedSourceIndex = -1;
  }

  source_pos_lookup->type |= type;
  ++scr::parser_glob_.currentSourcePosCount;
  opcode_lookup->sourcePosCount =
      static_cast<unsigned short>(scr::parser_glob_.currentSourcePosCount);
  ++scr::parser_glob_.opcodeLookupLen;
  ++scr::parser_glob_.sourcePosLookupLen;
}

void remove_opcode_pos() {
  if (!scr::developer_) {
    return;
  }

  if (game::scrCompilePub->developer_statement == 2) {
    assert(!game::scrVarPub->developer_script);
    return;
  }

  assert(scr::parser_glob_.opcodeLookup);
  assert(scr::parser_glob_.sourcePosLookup);
  assert(game::scrCompilePub->opcodePos);
  assert(scr::parser_glob_.sourcePosLookupLen);

  --scr::parser_glob_.sourcePosLookupLen;
  assert(scr::parser_glob_.opcodeLookupLen);

  --scr::parser_glob_.opcodeLookupLen;

  assert(scr::parser_glob_.currentSourcePosCount);

  --scr::parser_glob_.currentSourcePosCount;

  auto* opcode_lookup =
      &scr::parser_glob_.opcodeLookup[scr::parser_glob_.opcodeLookupLen];

  assert(scr::parser_glob_.currentCodePos == game::scrCompilePub->opcodePos);
  assert(opcode_lookup->sourcePosIndex +
             scr::parser_glob_.currentSourcePosCount ==
         scr::parser_glob_.sourcePosLookupLen);
  assert(opcode_lookup->codePos == (char*)scr::parser_glob_.currentCodePos);

  if (!scr::parser_glob_.currentSourcePosCount) {
    scr::parser_glob_.currentCodePos = nullptr;
  }

  opcode_lookup->sourcePosCount =
      static_cast<unsigned short>(scr::parser_glob_.currentSourcePosCount);
}

void add_thread_start_opcode_pos(const unsigned int source_pos) {
  if (!scr::developer_) {
    return;
  }

  if (game::scrCompilePub->developer_statement == 2) {
    assert(!game::scrVarPub->developer_script);
    return;
  }

  assert(scr::parser_glob_.threadStartSourceIndex >= 0);
  auto* source_pos_lookup =
      &scr::parser_glob_
           .sourcePosLookup[scr::parser_glob_.threadStartSourceIndex];
  source_pos_lookup->sourcePos = source_pos;

  assert(!source_pos_lookup->type);

  source_pos_lookup->type = 8;
  scr::parser_glob_.threadStartSourceIndex = -1;
}

void runtime_error_internal(int channel, const char* code_pos,
                            unsigned int index, const char* msg) {
  static_assert(offsetof(game::scrVmPub_t, function_count) == 0x8);
  static_assert(offsetof(game::scrVmPub_t, function_frame_start) == 0x20);

  assert(game::Scr_IsInOpcodeMemory(code_pos));

  game::Com_PrintError(channel,
                       "\n******* script runtime error *******\n%s: ", msg);
  scr::print_prev_code_pos(channel, code_pos, index);

  if (game::scrVmPub->function_count) {
    for (auto i = game::scrVmPub->function_count - 1; i >= 1; --i) {
      game::Com_PrintError(channel, "called from:\n");
      scr::print_prev_code_pos(
          game::CON_CHANNEL_DONT_FILTER,
          game::scrVmPub->function_frame_start[i].fs.pos,
          game::scrVmPub->function_frame_start[i].fs.localId == 0);
    }

    game::Com_PrintError(channel, "started from:\n");
    scr::print_prev_code_pos(game::CON_CHANNEL_DONT_FILTER,
                             game::scrVmPub->function_frame_start[0].fs.pos, 1);
  }

  *game::com_fixedConsolePosition = false;
  game::Com_PrintError(channel, "************************************\n");
}

void runtime_error(const char* code_pos, unsigned int index, const char* msg,
                   const char* dialog_message) {
  static_assert(offsetof(game::scrVmPub_t, terminal_error) == 0x16);

  bool abort_on_error;
  const char* dialog_message_separator;

  if (!scr::developer_) {
    assert(game::Scr_IsInOpcodeMemory(code_pos));

    if (!(*dvars::com_developer)->current.enabled) {
      return;
    }
  }

  if (game::scrVmPub->debugCode) {
    game::Com_Printf(game::CON_CHANNEL_PARSERSCRIPT, "%s\n", msg);
    if (!game::scrVmPub->terminal_error) {
      return;
    }
    goto error;
  }

  abort_on_error = game::scrVmPub->terminal_error;
  runtime_error_internal(abort_on_error ? game::CON_CHANNEL_LOGFILEONLY
                                        : game::CON_CHANNEL_PARSERSCRIPT,
                         code_pos, index, msg);
  if (abort_on_error) {
  error:
    if (!dialog_message) {
      dialog_message = "";
      dialog_message_separator = "";
    } else {
      dialog_message_separator = "\n";
    }

    game::Com_Error(
        game::scrVmPub->terminal_error ? game::ERR_SCRIPT_DROP
                                       : game::ERR_SCRIPT,
        "\x15script runtime error\n(see console for details)\n%s%s%s", msg,
        dialog_message_separator, dialog_message);
  }
}

void compile_error(unsigned int source_pos, const char* msg, ...) {
  char line[1024]{};
  char text[1024]{};
  int col;
  va_list argptr;

  va_start(argptr, msg);
  vsnprintf_s(text, _TRUNCATE, msg, argptr);
  va_end(argptr);

  if (game::scrVarPub->evaluate) {
    if (!game::scrVarPub->error_message) {
      game::scrVarPub->error_message = game::va("%s", text);
    }
  } else {
    game::Scr_ShutdownAllocNode();
    game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT, "\n");
    game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT,
                         "******* script compile error *******\n");

    if (!scr::developer_ || !scr::parser_pub_.sourceBuf) {
      game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT, "%s\n", text);
      line[0] = '\0';

      game::Com_Printf(game::CON_CHANNEL_PARSERSCRIPT,
                       "************************************\n");
      game::Com_Error(
          game::ERR_SCRIPT_DROP,
          "\x15"
          "script compile error\n%s\n%s\n(see console for details)\n",
          text, line);
    } else {
      assert(scr::parser_pub_.sourceBuf);

      game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT, "%s: ", text);
      scr::print_source_pos(game::CON_CHANNEL_PARSERSCRIPT,
                            scr::parser_pub_.scriptfilename,
                            scr::parser_pub_.sourceBuf, source_pos);
      const auto line_number = scr::get_line_info(scr::parser_pub_.sourceBuf,
                                                  source_pos, &col, line);
      game::Com_Error(
          game::ERR_SCRIPT_DROP,
          "\x15"
          "script compile error\n%s\n%s(%d):\n %s\n(see console for details)\n",
          text, scr::parser_pub_.scriptfilename, line_number, line);
    }
  }
}

void compile_error2(const char* code_pos, const char* msg, ...) {
  char line[1024]{};
  char text[1024]{};
  va_list argptr;

  assert(!game::scrVarPub->evaluate);
  assert(game::Scr_IsInOpcodeMemory(code_pos));

  game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT, "\n");
  game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT,
                       "******* script compile error *******\n");

  va_start(argptr, msg);
  vsnprintf_s(text, _TRUNCATE, msg, argptr);
  va_end(argptr);

  game::Com_PrintError(game::CON_CHANNEL_PARSERSCRIPT, "%s: ", text);

  scr::print_prev_code_pos(game::CON_CHANNEL_PARSERSCRIPT, code_pos, 0);

  game::Com_Printf(game::CON_CHANNEL_PARSERSCRIPT,
                   "************************************\n");

  scr::get_text_source_pos(scr::parser_pub_.sourceBuf, code_pos, line);

  game::Com_Error(game::ERR_SCRIPT_DROP,
                  "\x15"
                  "script compile error\n%s\n%s\n(see console for details)\n",
                  text, line);
}

void mt_reset_stub() {
  utils::hook::invoke<void>(0x4DF3A0);

  scr::init_opcode_lookup();
  hunk::init_debug_memory();
}

void sl_shutdown_system_stub(unsigned int user) {
  utils::hook::invoke<void>(0x4B1260, user);

  scr::shutdown_opcode_lookup();
  hunk::shutdown_debug_memory();
}

void __declspec(naked) emit_thread_internal_stub() {
  __asm {
    pushad;

    push [esp + 0x20 + 0x8] // sourcePos;
    call add_thread_start_opcode_pos;
    add esp, 0x4;

    popad;

    cmp ds:0x158BFB8, 2;

    push 0x612477;
    ret;
  }
}
} // namespace

class error final : public component_interface {
public:
  void post_load() override {
    std::vector<std::pair<std::size_t, void*>> patches;

    const auto p = [&patches](const std::size_t a, void* b) {
      patches.emplace_back(a, b);
    };

    utils::hook(0x43ACE0, scr::print_prev_code_pos, HOOK_JUMP)
        .install() // hook*
        ->quick();

    utils::hook(0x612470, emit_thread_internal_stub, HOOK_JUMP)
        .install() // hook*
        ->quick();
    utils::hook::nop(0x612470 + 5, 2);

    utils::hook(0x4767A0, compile_error, HOOK_JUMP)
        .install() // hook*
        ->quick();
    utils::hook::nop(0x4767A0 + 5, 5);

    p(0x4F362D, scr::settings_stub); // ??
    p(0x4F3642, scr::settings_stub); // ??
    p(0x6042FB, scr::settings_stub); // Com_Init_Try_Block_Function

    p(0x4B0F07, mt_reset_stub); // Scr_BeginLoadScripts

    p(0x4D690B, sl_shutdown_system_stub);

    p(0x4FFEAA, scr::load_script_internal_stub); // ScriptCompile
    p(0x46CDA9, scr::load_script_internal_stub); // Scr_LoadScript

    p(0x61602D, runtime_error); // VM_Notify
    p(0x6194B6, runtime_error); // VM_ExecuteInternal
    p(0x619FAE, runtime_error); // VM_Execute_0

    p(0x609D8C, compile_error2); // Scr_CheckAnimsDefined
    p(0x60AC86, compile_error2); // LinkThread
    p(0x60AC9D, compile_error2); // ^^
    p(0x60ACB2, compile_error2); // ^^

    p(0x6124FD, add_opcode_pos); // EmitThreadInternal
    p(0x612506, add_opcode_pos); // EmitThreadInternal

    p(0x6120E2, add_opcode_pos); // EmitStatement

    p(0x612043, add_opcode_pos); // EmitBinaryEqualsOperatorExpression
    p(0x612029, add_opcode_pos); // ^^

    p(0x611FAA, add_opcode_pos); // EmitDecStatement
    p(0x611F97, add_opcode_pos); // ^^

    p(0x611ECA, add_opcode_pos); // EmitIncStatement
    p(0x611EB7, add_opcode_pos); // ^^

    p(0x611DEA, add_opcode_pos); // EmitAssignmentStatement

    p(0x611AB9, add_opcode_pos); // EmitArrayVariableRef
    p(0x611AB1, add_opcode_pos); // ^^

    p(0x611625, add_opcode_pos); // EmitSwitchStatement

    p(0x61145A, add_opcode_pos); // EmitEndOnStatement
    p(0x61144E, add_opcode_pos); // ^^

    p(0x61135B, add_opcode_pos); // EmitNotifyStatement
    p(0x61134F, add_opcode_pos); // ^^
    p(0x611341, add_opcode_pos); // ^^
    p(0x61123B, add_opcode_pos); // ^^

    p(0x6110DC, add_opcode_pos); // EmitWaittillmatchStatement
    p(0x6110C5, add_opcode_pos); // ^^
    p(0x6110B7, add_opcode_pos); // ^^
    p(0x6110AB, add_opcode_pos); // ^^
    p(0x6110A3, add_opcode_pos); // ^^

    p(0x610EAE, add_opcode_pos); // EmitWaittillStatement
    p(0x610EA1, add_opcode_pos); // ^^
    p(0x610E95, add_opcode_pos); // ^^
    p(0x610E8D, add_opcode_pos); // ^^

    p(0x610CB2, add_opcode_pos);    // EmitForStatement
    p(0x610C9E, add_opcode_pos);    // ^^
    p(0x610AE9, add_opcode_pos);    // ^^
    p(0x610AB2, remove_opcode_pos); // ^^ (EmitOpcode inlined?)

    p(0x6107F9, add_opcode_pos);    // EmitWhileStatement
    p(0x6107E5, add_opcode_pos);    // ^^
    p(0x61069D, add_opcode_pos);    // ^^
    p(0x610666, remove_opcode_pos); // ^^ (EmitOpcode inlined?)

    p(0x610475, add_opcode_pos);    // EmitIfElseStatement
    p(0x6103F1, add_opcode_pos);    // ^^
    p(0x610359, add_opcode_pos);    // ^^
    p(0x6102CD, add_opcode_pos);    // ^^
    p(0x610296, remove_opcode_pos); // ^^ (EmitOpcode inlined?)

    p(0x6101B8, add_opcode_pos);    // EmitIfStatement
    p(0x610156, add_opcode_pos);    // ^^
    p(0x61011E, remove_opcode_pos); // ^^ (EmitOpcode inlined?)

    p(0x61006E, add_opcode_pos); // EmitWaitStatement
    p(0x610062, add_opcode_pos); // ^^
    p(0x61005A, add_opcode_pos); // ^^

    p(0x60FF89, add_opcode_pos); // EmitReturnStatement

    p(0x60FB40, add_opcode_pos); // EmitOrEvalVariableExpression

    p(0x60FA9F, add_opcode_pos); // ??

    p(0x60F9F0, add_opcode_pos); // ??
    p(0x60F9E8, add_opcode_pos); // ^^

    p(0x60F991, add_opcode_pos); // ??

    p(0x60F7C0, add_opcode_pos); // EmitContinueStatement

    p(0x60F690, add_opcode_pos); // EmitBreakStatement

    p(0x60F2B2, add_opcode_pos); // ??

    p(0x60F24A, add_opcode_pos); // EmitWaittillFrameEnd
    p(0x60F242, add_opcode_pos); // ^^

    p(0x60EFDE, add_opcode_pos); // ??

    p(0x60EEC7, add_opcode_pos); // ??

    p(0x60EDA7, add_opcode_pos); // ??

    p(0x60EC5B, add_opcode_pos); // EmitOrEvalPrimitiveExpressionList

    p(0x60EA1E, add_opcode_pos); // ??
    p(0x60E9AD, add_opcode_pos); // ^^

    p(0x60E5E2, add_opcode_pos); // ??

    p(0x60E595, add_opcode_pos); // ??

    p(0x60E4CD, add_opcode_pos); // ??

    p(0x60E3F5, add_opcode_pos); // ??

    p(0x60E270, add_opcode_pos); // ??
    p(0x60E24D, add_opcode_pos); // ^^

    p(0x60E210, add_opcode_pos); // ??
    p(0x60E1ED, add_opcode_pos); // ^^

    p(0x60E155, add_opcode_pos); // ??

    p(0x60DFB5, add_opcode_pos); // ??

    p(0x60DE13, add_opcode_pos);    // ??
    p(0x60DDDB, remove_opcode_pos); // ^^

    p(0x60DC83, add_opcode_pos);    // ??
    p(0x60DC4B, remove_opcode_pos); // ^^

    p(0x60DB09, add_opcode_pos); // ??
    p(0x60DAFD, add_opcode_pos); // ^^

    p(0x60D968, add_opcode_pos);    // ??
    p(0x60D8E7, remove_opcode_pos); // ^^
    p(0x60D832, remove_opcode_pos); // ^^

    p(0x60D787, add_opcode_pos); // EmitGetFunction
    p(0x60D6C7, add_opcode_pos); // ^^
    p(0x60D5A7, add_opcode_pos); // ^^

    p(0x60D3AF, add_opcode_pos); // EmitArrayVariable
    p(0x60D3A3, add_opcode_pos); // ^^

    p(0x60D33D, add_opcode_pos); // ??
    p(0x60D30B, add_opcode_pos); // ^^
    p(0x60D2E4, add_opcode_pos); // ^^

    p(0x60D1A9, add_opcode_pos); // ??

    utils::hook(0x60D109, add_opcode_pos, HOOK_JUMP)
        .install()               // hook*
        ->quick();               // EmitClearArray
    p(0x60D0F9, add_opcode_pos); // ^^

    p(0x60D051, add_opcode_pos); // ??

    p(0x60CFA6, add_opcode_pos); // EmitLocalVariableRef

    p(0x60CF50, add_opcode_pos); // EmitLocalVariableRef

    p(0x60CEB1, add_opcode_pos); // ??

    p(0x60CE11, add_opcode_pos); // ??

    p(0x60CD71, add_opcode_pos); // ??

    p(0x60CCC9, add_opcode_pos); // ??

    p(0x60CC19, add_opcode_pos); // ??

    p(0x60CB69, add_opcode_pos); // ??

    p(0x60CAB9, add_opcode_pos); // ??

    p(0x60CA09, add_opcode_pos); // ??

    p(0x60C964, add_opcode_pos); // ??

    p(0x60C8A1, add_opcode_pos); // ??

    p(0x60C801, add_opcode_pos); // ??

    p(0x60C761, add_opcode_pos); // ??

    p(0x60C6C0, add_opcode_pos); // ??

    p(0x60C0BD, remove_opcode_pos); // EmitOpcode
    p(0x60C07E, remove_opcode_pos); // ^^
    p(0x60C067, remove_opcode_pos); // ^^
    p(0x60C004, remove_opcode_pos); // ^^
    p(0x60BFE9, remove_opcode_pos); // ^^
    p(0x60BFBE, remove_opcode_pos); // ^^
    p(0x60BFA4, remove_opcode_pos); // ^^
    p(0x60BF8D, remove_opcode_pos); // ^^
    p(0x60BF6A, remove_opcode_pos); // ^^
    p(0x60BF54, remove_opcode_pos); // ^^
    p(0x60BEC7, remove_opcode_pos); // ^^
    p(0x60BEB0, remove_opcode_pos); // ^^
    p(0x60BE8F, remove_opcode_pos); // ^^
    p(0x60BE6E, remove_opcode_pos); // ^^

    p(0x60AB02, add_opcode_pos); // EmitFunction

    p(0x60A703, add_opcode_pos); // EmitGetFloat inlined
    p(0x60A6CE, add_opcode_pos); // EmitValue (?? inlined)

    p(0x60A653, add_opcode_pos); // EmitGetAnimation

    p(0x60A5F4, add_opcode_pos); // EmitGetVector

    p(0x60A582, add_opcode_pos); // EmitGetIString

    p(0x60A512, add_opcode_pos); // EmitGetString

    p(0x60A0FD, add_opcode_pos); // EmitGetInteger
    p(0x60A0D0, add_opcode_pos); // ^^
    p(0x60A09C, add_opcode_pos); // ^^
    p(0x60A069, add_opcode_pos); // ^^
    p(0x60A033, add_opcode_pos); // ^^
    p(0x60A00F, add_opcode_pos); // ^^

    for (const auto& patch : patches) {
      utils::hook(patch.first, patch.second, HOOK_CALL).install()->quick();
    }
  }
};
} // namespace gsc

REGISTER_COMPONENT(gsc::error)
