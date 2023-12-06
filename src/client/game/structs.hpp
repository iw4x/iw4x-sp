#pragma once

#pragma warning(push)
#pragma warning(disable : 4324)

namespace game {
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

struct scr_entref_t {
  unsigned __int16 entnum;
  unsigned __int16 classnum;
};

typedef void (*BuiltinMethod)(scr_entref_t);

typedef void (*BuiltinFunction)();

struct BuiltinMethodDef {
  const char* actionString;
  void (*actionFunc)(scr_entref_t);
  int type;
};

struct BuiltinFunctionDef {
  const char* actionString;
  void (*actionFunc)();
  int type;
};

struct OpcodeLookup {
  const char* codePos;
  unsigned int sourcePosIndex;
  unsigned __int16 sourcePosCount;
  unsigned __int16 cumulOffset;
  unsigned __int16* localVars;
  int profileTime;
  int profileBuiltInTime;
  int profileUsage;
};

static_assert(sizeof(OpcodeLookup) == 0x1C);

struct SourceLookup {
  unsigned int sourcePos;
  int type;
};

struct SaveSourceBufferInfo {
  char* buf;
  char* sourceBuf;
  int len;
};

struct scrParserGlob_t {
  OpcodeLookup* opcodeLookup;
  unsigned int opcodeLookupMaxSize;
  unsigned int opcodeLookupLen;
  SourceLookup* sourcePosLookup;
  unsigned int sourcePosLookupMaxSize;
  unsigned int sourcePosLookupLen;
  unsigned int sourceBufferLookupMaxSize;
  const char* currentCodePos;
  unsigned int currentSourcePosCount;
  SaveSourceBufferInfo* saveSourceBufferLookup;
  unsigned int saveSourceBufferLookupLen;
  int delayedSourceIndex;
  int threadStartSourceIndex;
};

static_assert(sizeof(scrParserGlob_t) == 0x34);

struct SourceBufferInfo {
  const char* codePos;
  char* buf;
  const char* sourceBuf;
  int len;
  int sortedIndex;
  bool archive;
  int time;
  int avgTime;
  int maxTime;
  float totalTime;
  float totalBuiltIn;
};

struct CodeOffsetMap {
  int type;
  unsigned int cumulOffset;
  int codeOffset;
  int sourcePos;
  int newCodeOffest;
};

struct scrParserPub_t {
  SourceBufferInfo* sourceBufferLookup;
  unsigned int sourceBufferLookupLen;
  const char* scriptfilename;
  const char* sourceBuf;
  CodeOffsetMap* codeOffsetMap;
  unsigned int codeOffsetMapLen;
  int useCodeOffsetMap;
};

static_assert(sizeof(scrParserPub_t) == 0x1C);

struct VariableStackBuffer {
  const char* pos;
  unsigned __int16 size;
  unsigned __int16 bufLen;
  unsigned __int16 localId;
  unsigned __int8 time;
  char buf[1];
};

union VariableUnion {
  int intValue;
  float floatValue;
  unsigned int stringValue;
  const float* vectorValue;
  const char* codePosValue;
  unsigned int pointerValue;
  VariableStackBuffer* stackValue;
  unsigned int entityOffset;
};

struct VariableValue {
  VariableUnion u;
  int type;
};

struct function_stack_t {
  const char* pos;
  unsigned int localId;
  unsigned int localVarCount;
  VariableValue* top;
  VariableValue* startTop;
};

struct function_frame_t {
  function_stack_t fs;
  int topType;
};

struct scrVmPub_t {
  unsigned int* localVars;
  VariableValue* maxstack;
  int function_count;
  function_frame_t* function_frame;
  VariableValue* top;
  bool debugCode;
  bool abort_on_error;
  bool terminal_error;
  unsigned int inparamcount;
  unsigned int outparamcount;
  function_frame_t function_frame_start[32];
  VariableValue stack[2048];
};

struct HunkUser {
  HunkUser* current;
  HunkUser* next;
  int maxSize;
  int end;
  int pos;
  const char* name;
  bool fixed;
  int type;
  unsigned __int8 buf[1];
};

static_assert(sizeof(HunkUser) == 0x24);

struct scrVarPub_t {
  const char* fieldBuffer;
  unsigned __int16 canonicalStrCount;
  bool developer_script;
  bool evaluate;
  const char* error_message;
  int error_index;
  unsigned int time;
  unsigned int timeArrayId;
  unsigned int pauseArrayId;
  unsigned int notifyArrayId;
  unsigned int objectStackId;
  unsigned int levelId;
  unsigned int gameId;
  unsigned int animId;
  unsigned int freeEntList;
  unsigned int tempVariable;
  unsigned int numScriptValues[2];
  bool bInited;
  unsigned __int16 savecount;
  unsigned __int16 savecountMark;
  unsigned int checksum;
  unsigned int entId;
  unsigned int entFieldName;
  HunkUser* programHunkUser;
  const char* programBuffer;
  const char* endScriptBuffer;
  unsigned __int16 saveIdMap[36864];
  unsigned __int16 saveIdMapRev[36864];
  bool bScriptProfile;
  float scriptProfileMinTime;
  bool bScriptProfileBuiltin;
  float scriptProfileBuiltinMinTime;
  unsigned int numScriptThreads;
  unsigned int numScriptObjects;
  const char* varUsagePos;
  int ext_threadcount;
  int totalObjectRefCount;
  volatile int totalVectorRefCount;
  unsigned int removeId;
};

static_assert(sizeof(scrVarPub_t) == 0x2408C);

struct scr_localVar_t {
  unsigned int name;
  unsigned int sourcePos;
};

struct scr_block_t {
  int abortLevel;
  int localVarsCreateCount;
  int localVarsPublicCount;
  int localVarsCount;
  unsigned __int8 localVarsInitBits[8];
  scr_localVar_t localVars[64];
};

struct scrCompilePub_t {
  int value_count;
  int far_function_count;
  unsigned int loadedscripts;
  unsigned int scriptsPos;
  unsigned int scriptsCount;
  unsigned int scriptsDefine;
  unsigned int builtinFunc;
  unsigned int builtinMeth;
  unsigned __int16 canonicalStrings[65536];
  const char* in_ptr;
  bool in_ptr_valid;
  const char* parseBuf;
  bool script_loading;
  bool allowedBreakpoint;
  int developer_statement;
  char* opcodePos;
  unsigned int programLen;
  int func_table_size;
  int func_table[1024];
  scr_block_t* pauseBlock;
};

struct CaseStatementInfo {
  unsigned int name;
  const char* codePos;
  unsigned int sourcePos;
  CaseStatementInfo* next;
};

struct BreakStatementInfo {
  const char* codePos;
  const char* nextCodePos;
  BreakStatementInfo* next;
};

struct ContinueStatementInfo {
  const char* codePos;
  const char* nextCodePos;
  ContinueStatementInfo* next;
};

struct PrecacheEntry {
  unsigned __int16 filename;
  bool include;
  unsigned int sourcePos;
};

union sval_u {
  int type;
  unsigned int stringValue;
  unsigned int idValue;
  float floatValue;
  int intValue;
  sval_u* node;
  unsigned int sourcePosValue;
  const char* codePosValue;
  const char* debugString;
  scr_block_t* block;
};

struct VariableCompileValue {
  VariableValue value;
  sval_u sourcePos;
};

struct scrCompileGlob_t {
  unsigned char* codePos;
  unsigned char* prevOpcodePos;
  unsigned int filePosId;
  unsigned int fileCountId;
  int cumulOffset;
  int prevCumulOffset;
  int maxOffset;
  int maxCallOffset;
  bool bConstRefCount;
  bool in_developer_thread;
  unsigned int developer_thread_sourcePos;
  bool firstThread[2];
  CaseStatementInfo* currentCaseStatement;
  bool bCanBreak;
  BreakStatementInfo* currentBreakStatement;
  bool bCanContinue;
  ContinueStatementInfo* currentContinueStatement;
  scr_block_t** breakChildBlocks;
  int* breakChildCount;
  scr_block_t* breakBlock;
  scr_block_t** continueChildBlocks;
  int* continueChildCount;
  bool forceNotCreate;
  PrecacheEntry* precachescriptList;
  VariableCompileValue value_start[32];
};

struct scr_animtree_t {
  void* anims;
};

struct scrAnimPub_t {
  unsigned int animtrees;
  unsigned int animtree_node;
  unsigned int animTreeNames;
  scr_animtree_t xanim_lookup[2][128];
  unsigned int xanim_num[2];
  unsigned int animTreeIndex;
  bool animtree_loading;
};

enum {
  SOURCE_TYPE_BREAKPOINT = 0x1,
  SOURCE_TYPE_CALL = 0x2,
  SOURCE_TYPE_CALL_POINTER = 0x4,
  SOURCE_TYPE_THREAD_START = 0x8,
  SOURCE_TYPE_BUILTIN_CALL = 0x10,
  SOURCE_TYPE_NOTIFY = 0x20,
  SOURCE_TYPE_GETFUNCTION = 0x40,
  SOURCE_TYPE_WAIT = 0x80,
};

enum {
  VAR_STRING = 0x2,
  VAR_FLOAT = 0x5,
  VAR_INTEGER = 0x6,
};

enum {
  CON_CHANNEL_DONT_FILTER = 0x0,
  CON_CHANNEL_ERROR = 0x1,
  CON_CHANNEL_GAMENOTIFY = 0x2,
  CON_CHANNEL_BOLDGAME = 0x3,
  CON_CHANNEL_SUBTITLE = 0x4,
  CON_CHANNEL_OBITUARY = 0x5,
  CON_CHANNEL_LOGFILEONLY = 0x6,
  CON_CHANNEL_CONSOLEONLY = 0x7,
  CON_CHANNEL_GFX = 0x8,
  CON_CHANNEL_SOUND = 0x9,
  CON_CHANNEL_FILES = 0xA,
  CON_CHANNEL_DEVGUI = 0xB,
  CON_CHANNEL_PROFILE = 0xC,
  CON_CHANNEL_UI = 0xD,
  CON_CHANNEL_CLIENT = 0xE,
  CON_CHANNEL_SERVER = 0xF,
  CON_CHANNEL_SYSTEM = 0x10,
  CON_CHANNEL_PLAYERWEAP = 0x11,
  CON_CHANNEL_AI = 0x12,
  CON_CHANNEL_ANIM = 0x13,
  CON_CHANNEL_PHYS = 0x14,
  CON_CHANNEL_FX = 0x15,
  CON_CHANNEL_LEADERBOARDS = 0x16,
  CON_CHANNEL_PARSERSCRIPT = 0x17,
  CON_CHANNEL_SCRIPT = 0x18,
  CON_CHANNEL_NETWORK = 0x19,

  CON_BUILTIN_CHANNEL_COUNT = 0x1A,
};

enum XAssetType {
  ASSET_TYPE_PHYSPRESET = 0,
  ASSET_TYPE_PHYSCOLLMAP = 1,
  ASSET_TYPE_XANIMPARTS = 2,
  ASSET_TYPE_XMODEL_SURFS = 3,
  ASSET_TYPE_XMODEL = 4,
  ASSET_TYPE_MATERIAL = 5,
  ASSET_TYPE_PIXELSHADER = 6,
  ASSET_TYPE_VERTEXSHADER = 7,
  ASSET_TYPE_VERTEXDECL = 8,
  ASSET_TYPE_TECHNIQUE_SET = 9,
  ASSET_TYPE_IMAGE = 10,
  ASSET_TYPE_SOUND = 11,
  ASSET_TYPE_SOUND_CURVE = 12,
  ASSET_TYPE_LOADED_SOUND = 13,
  ASSET_TYPE_CLIPMAP_SP = 14,
  ASSET_TYPE_CLIPMAP_MP = 15,
  ASSET_TYPE_COMWORLD = 16,
  ASSET_TYPE_GAMEWORLD_SP = 17,
  ASSET_TYPE_GAMEWORLD_MP = 18,
  ASSET_TYPE_MAP_ENTS = 19,
  ASSET_TYPE_FXWORLD = 20,
  ASSET_TYPE_GFXWORLD = 21,
  ASSET_TYPE_LIGHT_DEF = 22,
  ASSET_TYPE_UI_MAP = 23,
  ASSET_TYPE_FONT = 24,
  ASSET_TYPE_MENULIST = 25,
  ASSET_TYPE_MENU = 26,
  ASSET_TYPE_LOCALIZE_ENTRY = 27,
  ASSET_TYPE_WEAPON = 28,
  ASSET_TYPE_SNDDRIVER_GLOBALS = 29,
  ASSET_TYPE_FX = 30,
  ASSET_TYPE_IMPACT_FX = 31,
  ASSET_TYPE_AITYPE = 32,
  ASSET_TYPE_MPTYPE = 33,
  ASSET_TYPE_CHARACTER = 34,
  ASSET_TYPE_XMODELALIAS = 35,
  ASSET_TYPE_RAWFILE = 36,
  ASSET_TYPE_STRINGTABLE = 37,
  ASSET_TYPE_LEADERBOARD = 38,
  ASSET_TYPE_STRUCTURED_DATA_DEF = 39,
  ASSET_TYPE_TRACER = 40,
  ASSET_TYPE_VEHICLE = 41,
  ASSET_TYPE_ADDON_MAP_ENTS = 42,

  ASSET_TYPE_COUNT = 43,
};

enum FsThread {
  FS_THREAD_MAIN = 0x0,
  FS_THREAD_STREAM = 0x1,
  FS_THREAD_DATABASE = 0x2,
  FS_THREAD_BACKEND = 0x3,
  FS_THREAD_SERVER = 0x4,
  FS_THREAD_COUNT = 0x5,
  FS_THREAD_INVALID = 0x6,
};

enum FsListBehavior_e {
  FS_LIST_PURE_ONLY = 0x0,
  FS_LIST_ALL = 0x1,
};

enum fsMode_t {
  FS_READ = 0x0,
  FS_WRITE = 0x1,
  FS_APPEND = 0x2,
  FS_APPEND_SYNC = 0x3,
};

struct token_s {
  char string[1024];
  int type;
  int subtype;
  unsigned int intvalue;
  long double floatvalue;
  char* whitespace_p;
  char* endwhitespace_p;
  int line;
  int linescrossed;
  token_s* next;
};

static_assert(sizeof(token_s) == 0x430);

struct define_s {
  char* name;
  int flags;
  int builtin;
  int numparms;
  token_s* parms;
  token_s* tokens;
  define_s* next;
  define_s* hashnext;
};

struct punctuation_s {
  const char* p;
  int n;
  punctuation_s* next;
};

struct script_s {
  char filename[64];
  char* buffer;
  char* script_p;
  char* end_p;
  char* lastscript_p;
  char* whitespace_p;
  char* endwhitespace_p;
  int length;
  int line;
  int lastline;
  int tokenavailable;
  int flags;
  punctuation_s* punctuations;
  punctuation_s** punctuationtable;
  token_s token;
  script_s* next;
};

enum parseSkip_t {
  SKIP_NO = 0x0,
  SKIP_YES = 0x1,
  SKIP_ALL_ELIFS = 0x2,
};

struct indent_s {
  int type;
  parseSkip_t skip;
  script_s* script;
  indent_s* next;
};

struct source_s {
  char filename[64];
  char includepath[64];
  punctuation_s* punctuations;
  script_s* scriptstack;
  token_s* tokens;
  define_s* defines;
  define_s** definehash;
  indent_s* indentstack;
  int skip;
  token_s token;
};

struct pc_token_s {
  int type;
  int subtype;
  int intvalue;
  float floatvalue;
  char string[1024];
};

struct cmd_function_s {
  cmd_function_s* next;
  const char* name;
  const char* autoCompleteDir;
  const char* autoCompleteExt;
  void(__cdecl* function)();
  int flags;
};

struct CmdArgs {
  int nesting;
  int localClientNum[8];
  int controllerIndex[8];
  int argc[8];
  const char** argv[8];
};

struct msg_t {
  int overflowed;
  int readOnly;
  unsigned char* data;
  unsigned char* splitData;
  int maxsize;
  int cursize;
  int splitSize;
  int readcount;
  int bit;
  int lastEntityRef;
};

static_assert(sizeof(msg_t) == 0x28);

typedef enum {
  ERR_FATAL = 0x0,
  ERR_DROP = 0x1,
  ERR_SERVERDISCONNECT = 0x2,
  ERR_DISCONNECT = 0x3,
  ERR_SCRIPT = 0x4,
  ERR_SCRIPT_DROP = 0x5,
  ERR_LOCALIZATION = 0x6,
  ERR_MAPLOADERRORSUMMARY = 0x7,
} errorParm_t;

enum DvarFlags : std::uint16_t {
  DVAR_NONE = 0,
  DVAR_ARCHIVE = 1 << 0,
  DVAR_LATCH = 1 << 1,
  DVAR_CHEAT = 1 << 2,
  DVAR_CODINFO = 1 << 3,
  DVAR_SCRIPTINFO = 1 << 4,
  DVAR_TEMP = 1 << 5,
  DVAR_SAVED = 1 << 6,
  DVAR_INTERNAL = 1 << 7,
  DVAR_EXTERNAL = 1 << 8,
  DVAR_USERINFO = 1 << 9,
  DVAR_SERVERINFO = 1 << 10,
  DVAR_INIT = 1 << 11,
  DVAR_SYSTEMINFO = 1 << 12,
  DVAR_ROM = 1 << 13,
  DVAR_CHANGEABLE_RESET = 1 << 14,
  DVAR_AUTOEXEC = 1 << 15,
};

enum : std::int8_t {
  DVAR_TYPE_BOOL = 0x0,
  DVAR_TYPE_FLOAT = 0x1,
  DVAR_TYPE_FLOAT_2 = 0x2,
  DVAR_TYPE_FLOAT_3 = 0x3,
  DVAR_TYPE_FLOAT_4 = 0x4,
  DVAR_TYPE_INT = 0x5,
  DVAR_TYPE_ENUM = 0x6,
  DVAR_TYPE_STRING = 0x7,
  DVAR_TYPE_COLOR = 0x8,
  DVAR_TYPE_FLOAT_3_COLOR = 0x9,
};

enum DvarSetSource {
  DVAR_SOURCE_INTERNAL,
  DVAR_SOURCE_EXTERNAL,
  DVAR_SOURCE_SCRIPT,
  DVAR_SOURCE_DEVGUI,
};

union DvarValue {
  bool enabled;
  int integer;
  unsigned int unsignedInt;
  float value;
  float vector[4];
  const char* string;
  unsigned char color[4];
};

struct enum_limit {
  int stringCount;
  const char** strings;
};

struct int_limit {
  int min;
  int max;
};

struct float_limit {
  float min;
  float max;
};

union DvarLimits {
  enum_limit enumeration;
  int_limit integer;
  float_limit value;
  float_limit vector;
};

struct dvar_t {
  const char* name;
  const char* description;
  unsigned int flags;
  char type;
  bool modified;
  DvarValue current;
  DvarValue latched;
  DvarValue reset;
  DvarLimits domain;
  bool(__cdecl* domainFunc)(dvar_t*, DvarValue);
  dvar_t* hashNext;
};

enum netsrc_t {
  NS_CLIENT1 = 0x0,
  NS_SERVER = 0x1,
  NS_MAXCLIENTS = 0x1,
  NS_PACKET = 0x2,
};

enum netadrtype_t {
  NA_BOT = 0x0,
  NA_BAD = 0x1,
  NA_LOOPBACK = 0x2,
  NA_BROADCAST = 0x3,
  NA_IP = 0x4,
  NA_IPX = 0x5,
  NA_BROADCAST_IPX = 0x6,
};

struct netadr_t {
  netadrtype_t type;
  unsigned char ip[4];
  unsigned __int16 port;
  char ipx[10];
};

static_assert(sizeof(netadr_t) == 0x14);

struct Bounds {
  float midPoint[3];
  float halfSize[3];
};

struct TriggerModel {
  int contents;
  unsigned __int16 hullCount;
  unsigned __int16 firstHull;
};

struct TriggerHull {
  Bounds bounds;
  int contents;
  unsigned __int16 slabCount;
  unsigned __int16 firstSlab;
};

struct TriggerSlab {
  float dir[3];
  float midPoint;
  float halfSize;
};

struct MapTriggers {
  unsigned int count;
  TriggerModel* models;
  unsigned int hullCount;
  TriggerHull* hulls;
  unsigned int slabCount;
  TriggerSlab* slabs;
};

struct Stage {
  const char* name;
  float origin[3];
  unsigned __int16 triggerIndex;
  char sunPrimaryLightIndex;
};

struct MapEnts {
  const char* name;
  char* entityString;
  int numEntityChars;
  MapTriggers trigger;
  Stage* stages;
  char stageCount;
};

struct WeaponCompleteDef {
  const char* szInternalName;
}; // Incomplete

struct Font_s {
  const char* fontName;
  int pixelHeight;
  int glyphCount;
  void* material;
  void* glowMaterial;
  void* glyphs;
};

struct RawFile {
  const char* name;
  int compressedLen;
  int len;
  const char* buffer;
};

struct G_GlassPiece {
  unsigned __int16 damageTaken;
  unsigned __int16 collapseTime;
  int lastStateChangeTime;
  char impactDir;
  char impactPos[2];
};

static_assert(sizeof(G_GlassPiece) == 0xC);

struct G_GlassName {
  char* nameStr;
  unsigned __int16 name;
  unsigned __int16 pieceCount;
  unsigned __int16* pieceIndices;
};

static_assert(sizeof(G_GlassName) == 0xC);

struct G_GlassData {
  G_GlassPiece* glassPieces;
  unsigned int pieceCount;
  unsigned __int16 damageToWeaken;
  unsigned __int16 damageToDestroy;
  unsigned int glassNameCount;
  G_GlassName* glassNames;
  unsigned __int8 pad[108];
};

static_assert(sizeof(G_GlassData) == 0x80);

struct GameWorldMp {
  const char* name;
  G_GlassData* g_glassData;
};

struct GameWorldSp {
  const char* name;
  unsigned char __pad0[0x30];
  G_GlassData* g_glassData;
};

static_assert(sizeof(GameWorldSp) == 0x38);

struct StringTableCell {
  const char* string;
  int hash;
};

struct StringTable {
  const char* name;
  int columnCount;
  int rowCount;
  StringTableCell* values;
};

#define MAX_UISTRINGS 0x800

struct rectDef_s {
  float x;
  float y;
  float w;
  float h;
  char horzAlign;
  char vertAlign;
};

struct windowDef_t {
  const char* name;
  rectDef_s rect;
  rectDef_s rectClient;
  const char* group;
  int style;
  int border;
  int ownerDraw;
  int ownerDrawFlags;
  float borderSize;
  int staticFlags;
  int dynamicFlags[1];
  int nextTime;
  float foreColor[4];
  float backColor[4];
  float borderColor[4];
  float outlineColor[4];
  float disableColor[4];
  void* background;
};

static_assert(sizeof(windowDef_t) == 0xA4);

enum expDataType {
  VAL_INT = 0x0,
  VAL_FLOAT = 0x1,
  VAL_STRING = 0x2,
  NUM_INTERNAL_DATATYPES = 0x3,
  VAL_FUNCTION = 0x3,
  NUM_DATATYPES = 0x4,
};

struct Operand {
  expDataType dataType;
  char internals[4];
};

union entryInternalData {
  int op;
  Operand operand;
};

struct expressionEntry {
  int type;
  entryInternalData data;
};

struct menuTransition {
  int transitionType;
  int targetField;
  int startTime;
  float startVal;
  float endVal;
  float time;
  int endTriggerType;
};

struct StaticDvar {
  dvar_t* dvar;
  char* dvarName;
};

struct StaticDvarList {
  int numStaticDvars;
  StaticDvar** staticDvars;
};

struct StringList {
  int totalStrings;
  const char** strings;
};

struct ExpressionSupportingData; // required against my will

struct Statement_s {
  int numEntries;
  expressionEntry* entries;
  ExpressionSupportingData* supportingData;
  int lastExecuteTime;
  Operand lastResult;
};

struct UIFunctionList {
  int totalFunctions;
  Statement_s** functions;
};

struct ExpressionSupportingData {
  UIFunctionList uifunctions;
  StaticDvarList staticDvarList;
  StringList uiStrings;
};

static_assert(sizeof(Statement_s) == 0x18);

struct ItemFloatExpression {
  int target;
  Statement_s* expression;
};

struct MenuEventHandlerSet; // required against my will

struct ConditionalScript {
  MenuEventHandlerSet* eventHandlerSet;
  Statement_s* eventExpression;
};

static_assert(sizeof(ConditionalScript) == 0x8);

struct SetLocalVarData {
  const char* localVarName;
  Statement_s* expression;
};

union EventData {
  const char* unconditionalScript;
  ConditionalScript* conditionalScript;
  MenuEventHandlerSet* elseScript;
  SetLocalVarData* setLocalVarData;
};

#define MAX_EVENT_HANDLERS_PER_EVENT 200

enum EventType {
  EVENT_UNCONDITIONAL = 0x0,
  EVENT_IF = 0x1,
  EVENT_ELSE = 0x2,
  EVENT_SET_LOCAL_VAR_BOOL = 0x3,
  EVENT_SET_LOCAL_VAR_INT = 0x4,
  EVENT_SET_LOCAL_VAR_FLOAT = 0x5,
  EVENT_SET_LOCAL_VAR_STRING = 0x6,
  EVENT_COUNT = 0x7,
};

struct MenuEventHandler {
  EventData eventData;
  char eventType;
};

static_assert(sizeof(MenuEventHandler) == 0x8);

struct MenuEventHandlerSet {
  int eventHandlerCount;
  MenuEventHandler** eventHandlers;
};

struct ItemKeyHandler {
  int key;
  MenuEventHandlerSet* action;
  ItemKeyHandler* next;
};

struct columnInfo_s {
  int pos;
  int width;
  int maxChars;
  int alignment;
};

struct listBoxDef_s {
  int mousePos;
  int startPos[1];
  int endPos[1];
  int drawPadding;
  float elementWidth;
  float elementHeight;
  int elementStyle;
  int numColumns;
  columnInfo_s columnInfo[16];
  MenuEventHandlerSet* onDoubleClick;
  int notselectable;
  int noScrollBars;
  int usePaging;
  float selectBorder[4];
  void* selectIcon;
};

static_assert(sizeof(listBoxDef_s) == 0x144);

struct editFieldDef_s {
  float minVal;
  float maxVal;
  float defVal;
  float range;
  int maxChars;
  int maxCharsGotoNext;
  int maxPaintChars;
  int paintOffset;
};

static_assert(sizeof(editFieldDef_s) == 0x20);

#define MAX_MULTI_DVARS 32

struct multiDef_s {
  const char* dvarList[MAX_MULTI_DVARS];
  const char* dvarStr[MAX_MULTI_DVARS];
  float dvarValue[MAX_MULTI_DVARS];
  int count;
  int strDef;
};

static_assert(sizeof(multiDef_s) == 0x188);

struct newsTickerDef_s {
  int feedId;
  int speed;
  int spacing;
  int lastTime;
  int start;
  int end;
  float x;
};

static_assert(sizeof(newsTickerDef_s) == 0x1C);

struct textScrollDef_s {
  int startTime;
};

union itemDefData_t {
  listBoxDef_s* listBox;
  editFieldDef_s* editField;
  multiDef_s* multi;
  const char* enumDvarName;
  newsTickerDef_s* ticker;
  textScrollDef_s* scroll;
  void* data;
};

struct menuDef_t; // required against my will

struct itemDef_s {
  windowDef_t window;
  rectDef_s textRect[1];
  int type;
  int dataType;
  int alignment;
  int fontEnum;
  int textAlignMode;
  float textalignx;
  float textaligny;
  float textscale;
  int textStyle;
  int gameMsgWindowIndex;
  int gameMsgWindowMode;
  const char* text;
  int itemFlags;
  menuDef_t* parent;
  MenuEventHandlerSet* mouseEnterText;
  MenuEventHandlerSet* mouseExitText;
  MenuEventHandlerSet* mouseEnter;
  MenuEventHandlerSet* mouseExit;
  MenuEventHandlerSet* action;
  MenuEventHandlerSet* accept;
  MenuEventHandlerSet* onFocus;
  MenuEventHandlerSet* leaveFocus;
  const char* dvar;
  const char* dvarTest;
  ItemKeyHandler* onKey;
  const char* enableDvar;
  const char* localVar;
  int dvarFlags;
  void* focusSound;
  float special;
  int cursorPos[1];
  itemDefData_t typeData;
  int imageTrack;
  int floatExpressionCount;
  ItemFloatExpression* floatExpressions;
  Statement_s* visibleExp;
  Statement_s* disabledExp;
  Statement_s* textExp;
  Statement_s* materialExp;
  float glowColor[4];
  bool decayActive;
  int fxBirthTime;
  int fxLetterTime;
  int fxDecayStartTime;
  int fxDecayDuration;
  int lastSoundPlayedTime;
};

static_assert(sizeof(itemDef_s) == 0x17C);

struct menuDef_t {
  windowDef_t window;
  const char* font;
  int fullScreen;
  int itemCount;
  int fontIndex;
  int cursorItem[1];
  int fadeCycle;
  float fadeClamp;
  float fadeAmount;
  float fadeInAmount;
  float blurRadius;
  MenuEventHandlerSet* onOpen;
  MenuEventHandlerSet* onCloseRequest;
  MenuEventHandlerSet* onClose;
  MenuEventHandlerSet* onESC;
  ItemKeyHandler* onKey;
  Statement_s* visibleExp;
  const char* allowedBinding;
  const char* soundName;
  int imageTrack;
  float focusColor[4];
  Statement_s* rectXExp;
  Statement_s* rectYExp;
  Statement_s* rectWExp;
  Statement_s* rectHExp;
  Statement_s* openSoundExp;
  Statement_s* closeSoundExp;
  itemDef_s** items;
  menuTransition scaleTransition[1];
  menuTransition alphaTransition[1];
  menuTransition xTransition[1];
  menuTransition yTransition[1];
  ExpressionSupportingData* expressionData;
};

static_assert(sizeof(menuDef_t) == 0x190);

struct loadAssets_t {
  float fadeClamp;
  int fadeCycle;
  float fadeAmount;
  float fadeInAmount;
};

enum UILocalVarType {
  UILOCALVAR_INT = 0x0,
  UILOCALVAR_FLOAT = 0x1,
  UILOCALVAR_STRING = 0x2,
};

struct UILocalVar {
  UILocalVarType type;
  const char* name;
  union {
    int integer;
    float value;
    const char* string;
  } u;
};

struct UILocalVarContext {
  UILocalVar table[256];
};

struct UiContext {
  int localClientNum;
  float bias;
  int realTime;
  int frameTime;
  struct {
    float x;
    float y;
    int lastMoveTime;
  } cursor;
  int isCursorVisible;
  int paintFull;
  int screenWidth;
  int screenHeight;
  float screenAspect;
  float FPS;
  float blurRadiusOut;
  menuDef_t* Menus[640];
  int menuCount;
  menuDef_t* menuStack[16];
  int openMenuCount;
  UILocalVarContext localVars;
  const StringTable* cinematicSubtitles;
};

struct MenuList {
  const char* name;
  int menuCount;
  menuDef_t** menus;
};

struct savegameStatus_s {
  int sortKey;
  int sortDir;
  int displaySavegames[256];
};

struct qtime_s {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};

struct SavegameInfo {
  const char* savegameFile;
  const char* savegameName;
  const char* imageName;
  const char* mapName;
  const char* savegameInfoText;
  const char* time;
  const char* date;
  qtime_s tm;
};

struct uiInfo_s {
  UiContext uiDC;
  SavegameInfo savegameList[512];
  int savegameCount;
  savegameStatus_s savegameStatus;
  int timeIndex;
  int previousTimes[4];
  bool allowScriptMenuResponse;
  char savegameName[64];
  char savegameInfo[256];
  void* sshotImage;
  char sshotImageName[64];
};

static_assert(sizeof(uiInfo_s) == 0x9C2C);

struct LocalizeEntry {
  const char* value;
  const char* name;
};

union XAssetHeader {
  GameWorldSp* gameWorldSp;
  GameWorldMp* gameWorldMp;
  MapEnts* mapEnts;
  Font_s* font;
  MenuList* menuList;
  LocalizeEntry* localize;
  WeaponCompleteDef* weapon;
  RawFile* rawfile;
  StringTable* stringTable;
  void* data;
};

struct XAsset {
  XAssetType type;
  XAssetHeader header;
};

struct XAssetEntry {
  XAsset asset;
  unsigned char zoneIndex;
  volatile unsigned char inuseMask;
  bool printedMissingAsset;
  unsigned __int16 nextHash;
  unsigned __int16 nextOverride;
};

static_assert(sizeof(XAssetEntry) == 0x10);

struct XZoneInfo {
  const char* name;
  int allocFlags;
  int freeFlags;
};

struct playerState_s {
  int commandTime;
  int pm_type;
  int pm_time;
  int pm_flags;
  int otherFlags;
  int linkFlags;
  int bobCycle;
  float origin[3];
  float velocity[3];
  unsigned char __pad0[0xAC70];
};

static_assert(sizeof(playerState_s) == 0xACA4);

struct gclient_s {
  playerState_s ps;
  unsigned char __pad0[0xB0];
  int flags; // 0xAD54
  unsigned char __pad1[0x27C];
};

static_assert(sizeof(gclient_s) == 0xAFD4);

struct entityState_s {
  unsigned char eType; // 0
  unsigned char __pad0[0x7D];
  unsigned __int16 number; // 0x7E
  unsigned char __pad1[0x2C];
};

static_assert(sizeof(entityState_s) == 0xAC);

struct entityShared_t {
  unsigned char __pad0[0x5C];
};

static_assert(sizeof(entityShared_t) == 0x5C);

struct gentity_s {
  entityState_s s;
  entityShared_t r;
  gclient_s* client;
  unsigned char __pad0[0x164];
};

static_assert(sizeof(gentity_s) == 0x270);

struct client_t {
  unsigned char __pad0[0x1674];
};

static_assert(sizeof(client_t) == 0x1674);

struct level_locals_t {
  unsigned char __pad0[0x34];
  int time;
  unsigned char __pad1[0x4748];
};

static_assert(sizeof(level_locals_t) == 0x4780);

struct field_t {
  int cursor;
  int scroll;
  int drawWidth;
  int widthInPixels;
  float charHeight;
  int fixedSize;
  char buffer[256];
};

struct ConDrawInputGlob {
  char autoCompleteChoice[64];
  int matchIndex;
  int matchCount;
  const char* inputText;
  int inputTextLen;
  bool hasExactMatch;
  bool mayAutoComplete;
  float x;
  float y;
  float leftX;
  float fontHeight;
};

struct MessageLine {
  int messageIndex;
  int textBufPos;
  int textBufSize;
  int typingStartTime;
  int lastTypingSoundTime;
  int flags;
};

struct Message {
  int startTime;
  int endTime;
};

struct MessageWindow {
  MessageLine* lines;
  Message* messages;
  char* circularTextBuffer;
  int textBufSize;
  int lineCount;
  int padding;
  int scrollTime;
  int fadeIn;
  int fadeOut;
  int textBufPos;
  int firstLineIndex;
  int activeLineCount;
  int messageIndex;
};

struct MessageBuffer {
  char gamemsgText[4][2048];
  MessageWindow gamemsgWindows[4];
  MessageLine gamemsgLines[4][12];
  Message gamemsgMessages[4][12];
  char miniconText[4096];
  MessageWindow miniconWindow;
  MessageLine miniconLines[100];
  Message miniconMessages[100];
  char errorText[1024];
  MessageWindow errorWindow;
  MessageLine errorLines[5];
  Message errorMessages[5];
};

struct Console {
  MessageWindow consoleWindow;
  MessageLine consoleLines[1024];
  Message consoleMessages[1024];
  char consoleText[65536];
  char textTempLine[512];
  unsigned int lineOffset;
  int displayLineOffset;
  int prevChannel;
  bool outputVisible;
  int fontHeight;
  int visibleLineCount;
  int visiblePixelWidth;
  float screenMin[2];
  float screenMax[2];
  MessageBuffer messageBuffer[1];
  float color[4];
};

enum {
  KEYCATCH_CONSOLE = 0x1,
};

enum ScreenPlacementMode {
  SCRMODE_FULL = 0x0,
  SCRMODE_DISPLAY = 0x1,
  SCRMODE_INVALID = 0x2,
  SCRMODE_COUNT = 0x3,
};

struct ScreenPlacement {
  float scaleVirtualToReal[2];
  float scaleVirtualToFull[2];
  float scaleRealToVirtual[2];
  float realViewportPosition[2];
  float realViewportSize[2];
  float virtualViewableMin[2];
  float virtualViewableMax[2];
  float realViewableMin[2];
  float realViewableMax[2];
  float virtualAdjustableMin[2];
  float virtualAdjustableMax[2];
  float realAdjustableMin[2];
  float realAdjustableMax[2];
  float subScreenLeft;
};

enum usercmdButtonBits {
  CMD_BUTTON_LEAN_LEFT = 0x40,
  CMD_BUTTON_LEAN_RIGHT = 0x80,
};

struct kbutton_t {
  int down[2];
  unsigned int downtime;
  unsigned int msec;
  bool active;
  bool wasPressed;
};

struct usercmd_s {
  int serverTime;
  int buttons;
  int angles[3];
  unsigned __int16 weapon;
  unsigned __int16 primaryWeaponForAltMode;
  unsigned __int16 offHandIndex;
  char forwardmove;
  char rightmove;
  unsigned char upmove;
  unsigned char downmove;
  char pitchmove;
  char yawmove;
  float gunPitch;
  float gunYaw;
  float gunXOfs;
  float gunYOfs;
  float gunZOfs;
  float meleeChargeYaw;
  unsigned char meleeChargeDist;
  char selectedLoc[2];
  unsigned char selectedLocAngle;
  char remoteControlAngles[2];
};

struct weaponParms {
  float forward[3];
  float right[3];
  float up[3];
  float muzzleTrace[3];
  float gunForward[3];
  unsigned int weaponIndex;
  const void* weapDef;
  const void* weapCompleteDef;
};

struct lockonFireParms {
  bool lockon;
  gentity_s* target;
  float targetPosOrOffset[3];
  bool topFire;
};

enum TraceHitType {
  TRACE_HITTYPE_NONE = 0x0,
  TRACE_HITTYPE_ENTITY = 0x1,
  TRACE_HITTYPE_DYNENT_MODEL = 0x2,
  TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
  TRACE_HITTYPE_GLASS = 0x4,
};

struct trace_t {
  float fraction;
  float normal[3];
  int surfaceFlags;
  int contents;
  const char* material;
  TraceHitType hitType;
  unsigned __int16 hitId;
  unsigned __int16 modelIndex;
  unsigned __int16 partName;
  unsigned __int16 partGroup;
  bool allsolid;
  bool startsolid;
  bool walkable;
};

static_assert(sizeof(trace_t) == 0x2C);

struct pml_t {
  float forward[3];
  float right[3];
  float up[3];
  float frametime;
  int msec;
  int walking;
  int groundPlane;
  trace_t groundTrace;
  float impactSpeed;
  float previous_origin[3];
  float previous_velocity[3];
  unsigned int holdrand;
};

static_assert(sizeof(pml_t) == 0x80);

struct pmove_t {
  playerState_s* ps;
  usercmd_s cmd;
  usercmd_s oldcmd;
  int tracemask;
  int numtouch;
  int touchents[32];
  Bounds bounds;
  float xyspeed;
  int proneChange;
  int viewChangeTime;
  float viewChange;
  float fTorsoPitch;
  float fWaistPitch;
  unsigned char handler;
};

static_assert(sizeof(pmove_t) == 0x140);

enum ScopedCriticalSectionType {
  SCOPED_CRITSECT_NORMAL = 0x0,
  SCOPED_CRITSECT_DISABLED = 0x1,
  SCOPED_CRITSECT_RELEASE = 0x2,
  SCOPED_CRITSECT_TRY = 0x3,
};

enum CriticalSection {
  CRITSECT_CONSOLE = 0x0,
  CRITSECT_LSP = 0x1A,
  CRITSECT_CBUF = 0x21,
  CRITSECT_COUNT = 0x28,
};

enum {
  THREAD_VALUE_PROF_STACK = 0x0,
  THREAD_VALUE_VA = 0x1,
  THREAD_VALUE_COM_ERROR = 0x2,
  THREAD_VALUE_TRACE = 0x3,
  THREAD_VALUE_COUNT = 0x4,
};

struct TempPriority {
  void* threadHandle;
  int oldPriority;
};

struct FastCriticalSection {
  volatile long readCount;
  volatile long writeCount;
  TempPriority tempPriority;
};

struct ProfileAtom {
  unsigned int value[1];
};

volatile struct ProfileReadable {
  unsigned int hits;
  ProfileAtom total;
  ProfileAtom self;
};

struct ProfileWritable {
  int nesting;
  unsigned int hits;
  ProfileAtom start[3];
  ProfileAtom total;
  ProfileAtom child;
};

struct profile_t {
  ProfileWritable write;
  ProfileReadable read;
};

struct profile_guard_t {
  int id;
  profile_t** ppStack;
};

struct ProfileStack {
  profile_t prof_root;
  profile_t* prof_pStack[16384];
  profile_t** prof_ppStack;
  profile_t prof_array[443];
  ProfileAtom prof_overhead_internal;
  ProfileAtom prof_overhead_external;
  profile_guard_t prof_guardstack[32];
  int prof_guardpos;
  float prof_timescale;
};

struct va_info_t {
  char va_string[2][1024];
  int index;
};

static_assert(sizeof(va_info_t) == 0x804);

struct TraceCheckCount {
  int global;
  int* partitions;
};

struct TraceThreadInfo {
  TraceCheckCount checkcount;
};

static_assert(sizeof(TraceThreadInfo) == 0x8);
} // namespace game

#pragma warning(pop)
