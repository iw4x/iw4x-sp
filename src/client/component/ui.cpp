#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "botlib/l_precomp.hpp"

#include <utils/hook.hpp>

namespace ui {
namespace {
#define ITEM_TYPE_LISTBOX 6 // scrollable list
#define ITEM_TYPE_MULTI 12  // multiple list setting, enumerated

#define PARSE_FLOAT_TOKEN(name)                                                \
  if (!game::I_stricmp(token.string, #name)) {                                 \
    if (!game::PC_Float_Parse(handle, &g_load.loadAssets.##name##)) {          \
      return false;                                                            \
    }                                                                          \
    continue;                                                                  \
  }

#define PARSE_INT_TOKEN(name)                                                  \
  if (!game::I_stricmp(token.string, #name)) {                                 \
    if (!game::PC_Int_Parse(handle, &g_load.loadAssets.##name##)) {            \
      return false;                                                            \
    }                                                                          \
    continue;                                                                  \
  }

#define FREE_STATEMENT(statement)                                              \
  {                                                                            \
    if ((statement)) {                                                         \
      menu_free_expression_supporting_data((statement)->supportingData);       \
    }                                                                          \
    game::free_expression((statement));                                        \
  }

template <typename T, int N, int M> struct KeywordHashEntry {
  const char* keyword;
  int (*func)(T*, int);
};

KeywordHashEntry<game::menuDef_t, 128, 3523>** menu_parse_keyword_hash;

std::vector<game::menuDef_t*> loaded_menus_list;

struct {
  game::loadAssets_t loadAssets;
  game::MenuList menuList;
  game::itemDef_s* items[512];
  game::menuDef_t* menus[512];
} g_load;

char menu_buf[0x8000];

int asset_parse(int handle) {
  game::pc_token_s token{};

  if (!game::PC_ReadTokenHandle(handle, &token)) {
    return false;
  }

  if (game::I_stricmp(token.string, "{") != 0) {
    return false;
  }

  while (true) {
    if (!game::PC_ReadTokenHandle(handle, &token)) {
      return false;
    }

    if (!game::I_stricmp(token.string, ";")) {
      continue;
    }

    if (!game::I_stricmp(token.string, "}")) {
      return true;
    }

    PARSE_FLOAT_TOKEN(fadeClamp);
    PARSE_INT_TOKEN(fadeCycle);
    PARSE_FLOAT_TOKEN(fadeAmount);
    PARSE_FLOAT_TOKEN(fadeInAmount);

    game::PC_SourceError(
        handle,
        "Unknown token %s in assetGlobalDef.  Valid commands are 'fadeClamp', "
        "'fadeCycle', 'fadeAmount', and 'fadeInAmount'\n",
        token.string);
  }
}

void* alloc(int size, int alignment) {
  return game::Hunk_AllocAlignInternal(size, alignment);
}

template <int HASH_COUNT, int HASH_SEED>
int keyword_hash_key(const char* keyword) {
  auto hash = 0;
  for (auto i = 0; keyword[i]; ++i) {
    hash +=
        (i + HASH_SEED) * std::tolower(static_cast<unsigned char>(keyword[i]));
  }
  return (hash + (hash >> 8)) & (128 - 1);
}

template <typename T, int N, int M>
KeywordHashEntry<T, N, M>* keyword_hash_find(KeywordHashEntry<T, N, M>** table,
                                             const char* keyword) {
  auto hash = keyword_hash_key<N, M>(keyword);
  KeywordHashEntry<T, N, M>* key = table[hash];
  if (key && !game::I_stricmp(key->keyword, keyword)) {
    return key;
  }

  return nullptr;
}

int menu_parse(int handle, game::menuDef_t* menu) {
  game::pc_token_s token{};

  if (!game::PC_ReadTokenHandle(handle, &token))
    return false;

  if (*token.string != '{')
    return false;

  while (true) {
    std::memset(&token, 0, sizeof(game::pc_token_s));

    if (!game::PC_ReadTokenHandle(handle, &token)) {
      game::PC_SourceError(handle, "end of file inside menu\n");
      return false;
    }

    if (*token.string == '}') {
      return true;
    }

    auto* key = keyword_hash_find(menu_parse_keyword_hash, token.string);
    if (!key) {
      game::PC_SourceError(handle, "unknown menu keyword %s", token.string);
      continue;
    }

    if (!key->func(menu, handle)) {
      game::PC_SourceError(handle, "couldn't parse menu keyword %s",
                           token.string);
      return false;
    }
  }
}

void item_set_screen_coords([[maybe_unused]] int context_index,
                            game::itemDef_s* item, float x, float y,
                            int horz_align, int vert_align) {
  assert(item);

  if (!item) {
    return;
  }

  if (item->window.border) {
    x += item->window.borderSize;
    y += item->window.borderSize;
  }

  item->window.rect = item->window.rectClient;
  item->window.rect.x = item->window.rect.x + x;
  item->window.rect.y = item->window.rect.y + y;

  if (!item->window.rect.horzAlign && !item->window.rect.vertAlign) {
    item->window.rect.horzAlign = static_cast<char>(horz_align);
    item->window.rect.vertAlign = static_cast<char>(vert_align);
  }
}

game::rectDef_s* window_get_rect(game::windowDef_t* w) {
  assert(w);
  return &w->rect;
}

void menu_update_position(int context_index, game::menuDef_t* menu) {
  if (menu == nullptr) {
    return;
  }

  const auto* rect = window_get_rect(&menu->window);
  float x = rect->x;
  float y = rect->y;
  if (menu->window.border) {
    x += menu->window.borderSize;
    y += menu->window.borderSize;
  }

  for (int i = 0; i < menu->itemCount; ++i) {
    item_set_screen_coords(context_index, menu->items[i], x, y, rect->horzAlign,
                           rect->vertAlign);
  }
}

void menu_post_parse(game::menuDef_t* menu) {
  assert(menu);

  const auto item_count = 4 * menu->itemCount;
  menu->items = static_cast<game::itemDef_s**>(alloc(item_count, 4));
  std::memcpy(menu->items, g_load.items, item_count);

  if (menu->fullScreen) {
    menu->window.rect.x = 0.0f;
    menu->window.rect.y = 0.0f;
    menu->window.rect.w = 640.0f;
    menu->window.rect.h = 480.0f;
  }

  menu_update_position(0, menu);
}

void menu_set_cursor_item(int context_index, game::menuDef_t* menu,
                          int cursor_item) {
  assert(context_index < game::MAX_POSSIBLE_LOCAL_CLIENTS);
  assert(menu);
  menu->cursorItem[context_index] = cursor_item;
}

void window_init(game::windowDef_t* w) {
  std::memset(w, 0, sizeof(game::windowDef_t));
  w->borderSize = 1.0f;
  w->foreColor[0] = w->foreColor[1] = w->foreColor[2] = w->foreColor[3] = 1.0f;
}

void menu_init(game::menuDef_t* menu) {
  std::memset(menu, 0, sizeof(game::menuDef_t));
  menu_set_cursor_item(0, menu, -1);

  menu->fadeAmount = g_load.loadAssets.fadeAmount;
  menu->fadeInAmount = g_load.loadAssets.fadeInAmount;
  menu->fadeClamp = g_load.loadAssets.fadeClamp;
  menu->fadeCycle = g_load.loadAssets.fadeCycle;
  menu->items = g_load.items;

  window_init(&menu->window);
}

void menu_free_expression_supporting_data(
    game::ExpressionSupportingData* data) {

  if (!data) {
    return;
  }

  for (auto i = 0; i < data->uifunctions.totalFunctions; ++i) {
    auto* function = data->uifunctions.functions[i];
    FREE_STATEMENT(function);
  }

  data->uifunctions.totalFunctions = 0;
}

void menu_free_event_handler(game::MenuEventHandlerSet* event_handler) {
  if (!event_handler) {
    return;
  }

  for (auto i = 0; i < event_handler->eventHandlerCount; ++i) {
    auto* event = event_handler->eventHandlers[i];

    game::ConditionalScript* conditional_script;
    game::MenuEventHandlerSet* else_script;
    game::SetLocalVarData* local_var;

    switch (event->eventType) {
    case game::EVENT_IF:
      conditional_script = event->eventData.conditionalScript;
      menu_free_event_handler(conditional_script->eventHandlerSet);
      FREE_STATEMENT(conditional_script->eventExpression);
      break;
    case game::EVENT_ELSE:
      else_script = event->eventData.elseScript;
      menu_free_event_handler(else_script);
      break;
    case game::EVENT_SET_LOCAL_VAR_BOOL:
    case game::EVENT_SET_LOCAL_VAR_INT:
    case game::EVENT_SET_LOCAL_VAR_FLOAT:
    case game::EVENT_SET_LOCAL_VAR_STRING:
      local_var = event->eventData.setLocalVarData;
      FREE_STATEMENT(local_var->expression);
      break;
    default:
      break;
    }
  }

  event_handler->eventHandlerCount = 0;
}

void menu_free_item_key_handler(const game::ItemKeyHandler* key_handler) {
  while (key_handler) {
    menu_free_event_handler(key_handler->action);
    key_handler = key_handler->next;
  }
}

void menu_free_memory(game::menuDef_t* menu) {
  if (!menu) {
    return;
  }

  for (auto i = 0; i < menu->itemCount; ++i) {
    auto* item = menu->items[i];

    FREE_STATEMENT(item->visibleExp);
    FREE_STATEMENT(item->disabledExp);
    FREE_STATEMENT(item->textExp);
    FREE_STATEMENT(item->materialExp);

    menu_free_event_handler(item->mouseEnterText);
    menu_free_event_handler(item->mouseExitText);
    menu_free_event_handler(item->mouseEnter);
    menu_free_event_handler(item->mouseExit);
    menu_free_event_handler(item->action);
    menu_free_event_handler(item->accept);
    menu_free_event_handler(item->onFocus);
    menu_free_event_handler(item->leaveFocus);

    menu_free_item_key_handler(item->onKey);

    // free ItemFloatExpression*
    game::Menu_FreeItemMemory(item);

    if (item->dataType == ITEM_TYPE_LISTBOX) {
      menu_free_event_handler(item->typeData.listBox->onDoubleClick);
    }
  }

  menu->itemCount = 0;

  menu_free_event_handler(menu->onOpen);
  menu_free_event_handler(menu->onCloseRequest);
  menu_free_event_handler(menu->onClose);
  menu_free_event_handler(menu->onESC);

  menu_free_item_key_handler(menu->onKey);

  FREE_STATEMENT(menu->visibleExp);
  FREE_STATEMENT(menu->rectXExp);
  FREE_STATEMENT(menu->rectYExp);
  FREE_STATEMENT(menu->rectWExp);
  FREE_STATEMENT(menu->rectHExp);
  FREE_STATEMENT(menu->openSoundExp);
  FREE_STATEMENT(menu->closeSoundExp);

  // Our menu compiler code does not support parsing 'ui functions'
  if (!menu->expressionData) {
    return;
  }

  for (auto i = 0; i < menu->expressionData->uifunctions.totalFunctions; ++i) {
    auto* function_statement = menu->expressionData->uifunctions.functions[i];
    FREE_STATEMENT(function_statement);
  }

  menu->expressionData->uifunctions.totalFunctions = 0;
}

void menus_free_all_memory(game::UiContext* dc) {
  for (auto menu = 0; menu < dc->menuCount; ++menu) {
    menu_free_memory(dc->Menus[menu]);
  }
}

bool menu_new(int handle) {
  auto* menu = static_cast<game::menuDef_t*>(alloc(sizeof(game::menuDef_t), 4));
  menu_init(menu);

  if (!menu_parse(handle, menu)) {
    menu_free_memory(menu);
    return false;
  }

  if (!menu->window.name) {
    game::PC_SourceError(handle, "menu has no name");
    menu_free_memory(menu);
    return false;
  }

  menu_post_parse(menu);
  if (static_cast<std::size_t>(g_load.menuList.menuCount) >=
      std::extent_v<decltype(g_load.menus)>) {
    game::Com_Error(game::ERR_DROP, "\x15"
                                    "Menu_New: "
                                    "\x14"
                                    "EXE_ERR_OUT_OF_MEMORY");
  }

  g_load.menuList.menus[g_load.menuList.menuCount++] = menu;

  loaded_menus_list.emplace_back(menu);

  return true;
}

bool parse_menu_internal(const char* menu_file) {
  const char* builtin_defines[2];
  game::pc_token_s token;

  builtin_defines[0] = "PC";
  builtin_defines[1] = nullptr;

  game::Com_Printf(game::CON_CHANNEL_UI, "\tLoading '%s'...\n", menu_file);

  const auto handle = pc::load_source_handle(menu_file, builtin_defines);
  if (!handle) {
    game::Com_PrintError(game::CON_CHANNEL_UI, "Couldn't find menu file '%s'\n",
                         menu_file);
    return false;
  }

  while (game::PC_ReadTokenHandle(handle, &token)) {
    if (!game::I_stricmp(token.string, "}") ||
        !game::I_stricmp(token.string, "{")) {
      continue;
    }

    if (!game::I_stricmp(token.string, "assetGlobalDef")) {
      asset_parse(handle);
      continue;
    }

    if (!game::I_stricmp(token.string, "menudef")) {
      menu_new(handle);
      continue;
    }

    game::PC_SourceError(handle,
                         "Unknown token %s in menu file.  Expected \"menudef\" "
                         "or \"assetglobaldef\".\n",
                         token.string);
  }

  pc::free_source_handle(handle);
  return true;
}

int load_menu(const char** p) {
  if (*game::Com_Parse(p) != '{') {
    return false;
  }

  for (;;) {
    const auto* token = game::Com_Parse(p);
    if (!game::I_stricmp(token, "}")) {
      return true;
    }

    if (!token || !*token) {
      break;
    }

    parse_menu_internal(token);
  }

  return false;
}

game::MenuList* load_menus_load_obj(const char* menu_file) {
  std::memset(&g_load, 0, sizeof(g_load));
  g_load.menuList.menus = g_load.menus;

  auto f = 0;
  auto len = game::FS_FOpenFileRead(menu_file, &f);

  if (!f) {
    game::Com_PrintWarning(game::CON_CHANNEL_UI,
                           "WARNING: menu file not found: %s\n", menu_file);
#ifdef UI_LOAD_DEFAULT_MENU
    len = game::FS_FOpenFileByMode("ui/default.menu", &f, game::FS_READ);
    if (!f) {
      game::Com_Error(game::ERR_SERVERDISCONNECT,
                      "\x15"
                      "default.menu file not found. This is a default menu "
                      "that you should have.\n");
      return nullptr;
    }
#else
    return nullptr;
#endif
  }

  if (static_cast<std::size_t>(len) >= sizeof(menu_buf)) {
    game::FS_FCloseFile(f);
    game::Com_Error(game::ERR_SERVERDISCONNECT,
                    "\x15^1menu file too large: %s is %i, max allowed is %i",
                    menu_file, len, sizeof(menu_buf));
  }

  game::FS_Read(menu_buf, len, f);
  menu_buf[len] = '\0';
  game::FS_FCloseFile(f);

  game::Com_Compress(menu_buf);

  const auto* p = menu_buf;
  game::Com_BeginParseSession(menu_file);

  for (auto* token = game::Com_Parse(&p); token; token = game::Com_Parse(&p)) {
    if (!*token || *token == '}' || !game::I_stricmp(token, "}") ||
        !game::I_stricmp(token, "loadmenu") && !load_menu(&p)) {
      break;
    }
  }

  game::Com_EndParseSession();
  return &g_load.menuList;
}

game::MenuList* load_menu_load_obj(const char* menu_file) {
  std::memset(&g_load, 0, sizeof(g_load));
  g_load.menuList.menus = g_load.menus;

  if (!parse_menu_internal(menu_file)) {
    game::Com_PrintWarning(game::CON_CHANNEL_UI,
                           "WARNING: menu file not found: %s\n", menu_file);

#ifdef UI_LOAD_DEFAULT_MENU
    if (!parse_menu_internal("ui/default.menu")) {
      game::Com_Error(game::ERR_SERVERDISCONNECT,
                      "\x15"
                      "default.menu file not found. This is a default menu "
                      "that you should have.\n");
    }
#else
    return nullptr;
#endif
  }

  return &g_load.menuList;
}

game::MenuList* load_menus_fast_file(const char* menu_file) {
  return game::DB_FindXAssetHeader(game::ASSET_TYPE_MENULIST, menu_file)
      .menuList;
}

game::MenuList* load_menus_stub(const char* menu_file) {
  auto* menu_list = load_menus_load_obj(menu_file);
  if (!menu_list) {
    menu_list = load_menus_fast_file(menu_file);
  }

  return menu_list;
}

game::MenuList* load_menu_fast_file(const char* menu_file) {
  return game::DB_FindXAssetHeader(game::ASSET_TYPE_MENULIST, menu_file)
      .menuList;
}

game::MenuList* load_menu_stub(const char* menu_file) {
  auto* menu_list = load_menu_load_obj(menu_file);
  if (!menu_list) {
    menu_list = load_menu_fast_file(menu_file);
  }

  return menu_list;
}

void snd_fade_all_sounds_stub(float volume, int fade_time) {
  utils::hook::invoke<void>(0x4206A0, volume, fade_time);

  for (auto& menu : loaded_menus_list) {
    menu_free_memory(menu);
  }

  loaded_menus_list.clear();
}

void ui_shutdown_stub() {
  for (auto& menu : loaded_menus_list) {
    menu_free_memory(menu);
  }

  loaded_menus_list.clear();

  utils::hook::invoke<void>(0x4CBD70);
}
} // namespace

class component final : public component_interface {
public:
  static_assert(offsetof(game::UiContext, menuCount) == 0xA38);
  static_assert(offsetof(game::UiContext, Menus) == 0x38);

  static_assert(offsetof(game::menuDef_t, itemCount) == 0xAC);

  static_assert(offsetof(game::itemDef_s, dataType) == 0xBC);
  static_assert(offsetof(game::itemDef_s, typeData) == 0x134);
  static_assert(offsetof(game::itemDef_s, floatExpressionCount) == 0x13C);
  static_assert(offsetof(game::itemDef_s, floatExpressions) == 0x140);

  void post_load() override {
    menu_parse_keyword_hash =
        reinterpret_cast<KeywordHashEntry<game::menuDef_t, 128, 3523>**>(
            0x1933DB0);

    patch_sp();
  }

  void pre_destroy() override { assert(loaded_menus_list.empty()); }

private:
  static void patch_sp() {
    utils::hook(0x62DDD0, load_menus_stub, HOOK_JUMP).install()->quick();
    utils::hook::nop(0x62DDD0 + 5, 3);

    utils::hook(0x621194, load_menu_stub, HOOK_CALL).install()->quick();

    // Remove the menus from memory (CG_Shutdown)
    utils::hook(0x447905, snd_fade_all_sounds_stub, HOOK_CALL)
        .install() // hook*
        ->quick();
    // Remove the menus from memory (UI_Shutdown)
    utils::hook(0x4F4C4F, ui_shutdown_stub, HOOK_CALL)
        .install() // hook*
        ->quick();
  }
};
} // namespace ui

REGISTER_COMPONENT(ui::component)
