#pragma once

#define WEAK __declspec(selectany)

namespace game {
WEAK symbol<void(int channel, const char* fmt, ...)> Com_Printf{0x41BD20};
WEAK symbol<void(int channel, const char* fmt, ...)> Com_PrintError{0x4C6980};
WEAK symbol<void(int channel, const char* fmt, ...)> Com_DPrintf{0x42B1F0};
WEAK symbol<void(errorParm_t code, const char* fmt, ...)> Com_Error{0x43DD90};

WEAK symbol<int(int localClientNum)> CL_IsCgameInitialized{0x4EEA50};

WEAK symbol<WeaponCompleteDef*(const char* name, const char* folder)>
    BG_LoadWeaponVariantDefInternal{0x4F5AF0};

WEAK symbol<void(int, const char* text)> Cbuf_AddText{0x4A1090};
WEAK symbol<void(int localClientNum, int controllerIndex, const char* text)>
    Cmd_ExecuteSingleCommand{0x46AFD0};
WEAK symbol<void(const char* cmdName, void(*function),
                 cmd_function_t* allocedCmd)>
    Cmd_AddCommand{0x4478A0};
WEAK symbol<void(const char* cmdName, const char* dir, const char* ext)>
    Cmd_SetAutoComplete{0x48A880};

WEAK symbol<dvar_t*(const char* dvarName)> Dvar_FindVar{0x4B29D0};
WEAK symbol<const dvar_t*(const char* dvarName, const char* value,
                          unsigned __int16 flags, const char* description)>
    Dvar_RegisterString{0x49E0B0};
WEAK symbol<const dvar_t*(const char* dvarName, bool value,
                          unsigned __int16 flags, const char* description)>
    Dvar_RegisterBool{0x429390};
WEAK symbol<const dvar_t*(const char* dvarName, const char** valueList,
                          int defaultIndex, unsigned __int16 flags,
                          const char* description)>
    Dvar_RegisterEnum{0x4CB7C0};
WEAK symbol<const dvar_t*(const char* dvarName, float value, float min,
                          float max, unsigned __int16 flags,
                          const char* description)>
    Dvar_RegisterFloat{0x4051D0};
WEAK symbol<const dvar_t*(const char* dvarName, int value, int min, int max,
                          unsigned __int16 flags, const char* description)>
    Dvar_RegisterInt{0x4E9490};
WEAK symbol<void(const dvar_t* dvar, const char* value)> Dvar_SetString{
    0x480E70};

// Script
WEAK symbol<void(const char* error)> Scr_Error{0x4E9C50};
WEAK symbol<void(const char* error)> Scr_ObjectError{0x470600};

WEAK symbol<unsigned int()> Scr_GetNumParam{0x4443F0};
WEAK symbol<const char*(unsigned int index)> Scr_GetString{0x4D39C0};
WEAK symbol<unsigned int(unsigned int index)> Scr_GetConstString{0x4AF1B0};
WEAK symbol<int(unsigned int index)> Scr_GetInt{0x454520};
WEAK symbol<void(int value)> Scr_AddInt{0x4865B0};
WEAK symbol<void(int func, const char* name)> Scr_RegisterFunction{0x4F59C0};

WEAK symbol<char*(const char* filename, const char* extFilename,
                  const char* codePos, bool archive)>
    Scr_AddSourceBuffer{0x4173C0};

// SL
WEAK symbol<const char*(unsigned int stringValue)> SL_ConvertToString{0x40E990};

WEAK symbol<char*(netadr_t a)> NET_AdrToString{0x4BF490};

// Memory
WEAK symbol<void*(int size)> Hunk_AllocateTempMemory{0x492DF0};

// DB
WEAK symbol<void(XZoneInfo* zoneInfo, unsigned int zoneCount,
                 unsigned int syncMode)>
    DB_LoadXAssets{0x4E5930};
WEAK symbol<XAssetHeader(XAssetType type, const char* name)>
    DB_FindXAssetHeader{0x40B200};
WEAK symbol<int(XAssetType type, const char* name)> DB_IsXAssetDefault{
    0x41AB70};
WEAK symbol<void(RawFile* rawfile, char* buffer, int size)> DB_GetRawBuffer{
    0x4345E0};

// FS
WEAK symbol<void(void* buffer)> FS_FreeFile{0x4416B0};
WEAK symbol<unsigned int(void* ptr, unsigned int len, void* stream)> FS_Read{
    0x42EDC0};
WEAK symbol<void(void* h)> FS_FCloseFile{0x44E0A0};
WEAK symbol<int(const char* filename, void** file)> FS_FOpenFileRead{0x48DD10};
WEAK symbol<void(const char* gameName)> FS_Startup{0x47AF20};

// IW functions, could use Microsoft specific functions but who cares
WEAK symbol<int(const char* s0, const char* s1)> I_stricmp{0x409B80};
WEAK symbol<int(const char* s0, const char* s1, int n)> I_strnicmp{0x491E60};

WEAK symbol<void(field_t* edit)> Field_Clear{0x45C350};

// Variables
WEAK symbol<CmdArgs> cmd_args{0x144FED0};
WEAK symbol<CmdArgs> sv_cmd_args{0x145ABA0};
WEAK symbol<gentity_s> g_entities{0xEAAC38};
WEAK symbol<gclient_s> g_clients{0x10911E8};

WEAK symbol<bool> cin_skippable{0x73264C};

WEAK symbol<field_t> g_consoleField{0x88C700};
WEAK symbol<ConDrawInputGlob> conDrawInputGlob{0x86E788};
WEAK symbol<Console> con{0x86ED88};
WEAK symbol<float> g_console_char_height{0x732658};
WEAK symbol<int> g_console_field_width{0x732654};

WEAK symbol<void*> DB_GetXAssetSizeHandlers{0x733408};
WEAK symbol<void*> DB_XAssetPool{0x7337F8};
WEAK symbol<unsigned int> g_poolSize{0x733510};
} // namespace game
