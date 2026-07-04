#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "raylib.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdbool.h> 
#include <stddef.h>


// --- PICO_RAY OS ---
#define PICO_RAY_OS_NAME "PICO-RAY OS"
#define PICO_RAY_OS_VER "0.1.0"

// --- SYSTEM CONFIGURATION LIMITS ---
#define MAX_CARTRIDGE_PATH_LEN 256
#define MAX_FILE_DIALOG_FILES 32
#define MAX_FONT_CACHE_CODEPOINTS 1024
#define MAX_GLYPH_TABLES_PER_FONT 8
#define FONT_VIEWER_MAX_GLYPH_ROWS 16
#define MAX_SHELL_COMMAND_LEN 64
#define MAX_SYSTEM_MENUS 8

// --- PLUGIN ABI ---
#define PR_PLUGIN_ABI_VERSION 1
#define PR_PLUGIN_ABI_SYMBOL "PR_PLUGIN_ABI"
#define PR_PLUGIN_APP_SYMBOL "PR_PLUGIN_APP"

// --- DEBUG Identifier Constants ---

typedef enum {
    DEBUG_STAGE_LIST = 0,
    DEBUG_STAGE_DATA
} DebugStage;

typedef enum {
    DEBUG_TARGET_KERNEL = 0,
    DEBUG_TARGET_MOUSE,
    DEBUG_TARGET_META,
    DEBUG_TARGET_CART_RAM,
    DEBUG_TARGET_GRAPHICS,
    DEBUG_TARGET_EDITOR,
    DEBUG_TARGET_COUNT
} DebugTarget;

enum {
    SYS_APP_NONE = 0,
    SYS_APP_HOME,
    SYS_APP_FONT_STUDIO,    // Built-in Font Studio
    SYS_APP_LUA_RUNNER,     // Built-in Lua Runner
    SYS_APP_SPRITE_EDITOR,  // Built-in Sprite Editor
    SYS_APP_MAP_EDITOR,     // Built-in Map Editor
    SYS_APP_TERMINAL,       // Built-in Terminal / Shell
    SYS_APP_COUNT
};

// --- COLOR Identifier Constants ---
enum {
    P8_BLACK = 0,      P8_DARK_BLUE,     P8_DARK_PURPLE,     P8_DARK_GREEN,
    P8_BROWN,          P8_DARK_GREY,     P8_LIGHT_GREY,      P8_WHITE,
    P8_RED,            P8_ORANGE,        P8_YELLOW,          P8_GREEN,
    P8_BLUE,           P8_LAVENDER,      P8_PINK,            P8_LIGHT_PEACH,
    P8_EXT_BLACK,      P8_EXT_DARK_BLUE, P8_EXT_DARK_PURPLE, P8_EXT_TEAL,
    P8_EXT_BROWN,      P8_EXT_MUD,       P8_EXT_SAND,        P8_EXT_LEMON,
    P8_EXT_CRIMSON,    P8_EXT_FLAME,     P8_EXT_LIME,        P8_EXT_BRIGHT_GREEN,
    P8_EXT_ROYAL_BLUE, P8_EXT_MAUVE,     P8_EXT_CORAL,       P8_EXT_PEACH
};

// --- C64 ---
enum {
    C64_BLACK = 0, C64_BLUE,           C64_PURPLE,        C64_GREEN,
    C64_BROWN,     C64_DARK_GREY,      C64_GREY,          C64_WHITE,
    C64_RED,       C64_ORANGE,         C64_YELLOW,        C64_LIGHT_GREEN,
    C64_CYAN,      C64_LIGHT_BLUE,     C64_LIGHT_RED,     C64_LIGHT_GREY,
    C64_SAT_BLACK, C64_SAT_BLUE,       C64_SAT_PURPLE,    C64_SAT_GREEN,
    C64_SAT_BROWN, C64_SAT_DARK_GREY,  C64_SAT_GREY,      C64_SAT_WHITE,
    C64_SAT_RED,   C64_SAT_ORANGE,     C64_SAT_YELLOW,    C64_SAT_LIGHT_GREEN,
    C64_SAT_CYAN,  C64_SAT_LIGHT_BLUE, C64_SAT_LIGHT_RED, C64_SAT_LIGHT_GREY
};

// --- FONT Identifier Constants ---
enum {
    FONT_PICORAY = 0,  // Your own custom beautiful baseline font (8x6)
    FONT_PICO8,        // Official PICO-8 layout with symbols and katakana (8x6)
    FONT_PICORAY_UTF8,
    FONT_QUARTZ,
    FONT_PR_SMALL,     // Ultra-compact 3x5 / 4x5 micro font for tight spaces
    FONT_COUNT         // Tracks the total number of built-in system fonts
};

// --- ICON Identifier Constants ---
enum {
    ICON_PICORAY = 0,
    ICON_APPLE,
    ICON_BRUSH,
    ICON_CLOSE,
    ICON_DOWN,
    ICON_EMPTY,
    ICON_FLOOD,
    ICON_FONT_STUDIO,
    ICON_HOME,
    ICON_INFO,
    ICON_PRINT_LEFT,
    ICON_PRINT_RIGHT,
    ICON_FOLDER,
    ICON_LEFT,
    ICON_LUA,
    ICON_PAUSE,
    ICON_PLAY,
    ICON_PENCIL,
    ICON_PLUGIN,
    ICON_RELOAD,
    ICON_RIGHT,
    ICON_QUIT,
    ICON_SAVE,
    ICON_SPRITE_EDITOR,
    ICON_STOP,
    ICON_TERMINAL,
    ICON_TOOLS,
    ICON_UP,
    ICON_COUNT  // Tracks the total number of built-in icons
};

// --- LUA SECTION Identifier Constants ---
enum {
    SECTION_NONE = 0,
    SECTION_META,
    SECTION_GFX,
    SECTION_MAP,
    SECTION_LUA
};

// --- PALETTE Identifier Constants ---
enum {
    PALETTE_PICO8 = 0,
    PALETTE_PYXEL,
    PALETTE_C64,
    PALETTE_SONY_TRINITRON,
    PALETTE_COUNT  // Tracks the total number of built-in palettes
    // PALETTE_CGA,
};

// --- THEME Identifier Constants ---
enum {
    THEME_LIGHT = 0,     // Classic retro desktop OS look (Light Gray)
    THEME_DARK,          // Dark night-friendly theme (Dark Blue/Purple)
    THEME_HACKER_GREEN,  // Retro terminal aesthetic (Black and Green)
    THEME_COUNT          // Tracks the total number of built-in UI themes
};

// High-level operational modes for our unified filesystem interface row
typedef enum {
    FILE_DIALOG_OPEN,
    FILE_DIALOG_SAVE
} FileDialogMode;

// --- Type definition of Vector2
// typedef struct Vector2 {
//     float x;
//     float y;
// } Vector2;

// Dedicated state capsule managing the active audio device mixing matrix
typedef struct {
    Music backgroundMusic;      // Active streaming stream reference node
    bool isMusicActive;         // Replaced volatile static isMusicPlaying flag!
    float masterVolume;         // Cached processed floating point volume level (0.0f - 1.0f)
    
    // Future expansion slots ready for chiptune & tinySID emulators context data pointers:
    // void *synthContext;
    // int activeChiptuneChannels;
} AudioSystem;

typedef struct {
    Texture2D virtualScreenTexture; // GPU target texture sync layer
    Image fontAtlas;                // Stencil alpha-mask cache for blitting text
    Image iconAtlas;                // Stencil alpha-mask cache for blitting all system icons
    Image virtualVRAM;              // CPU-side software framebuffer image on RAM
    int  screenScale;               // Display multiplier zoom factor (e.g., 5)
    int  virtualWidth;              // Virtual screen width in pixels
    int  virtualHeight;             // Virtual screen height in pixels
    int  currentFontId;
    int  currentPaletteId;
    int  currentThemeId;
    int  systemFontId;
    bool isColorTransparent[32];  // Transparent color
} GraphicsSystem;

typedef struct {
    bool isAppLoaded;  // Tracks if a game cartridge or runner sub-module is currently active inside VRAM memory space
    bool isAboutOpen;
    bool isDebugOpen;    
    bool isPauseMenuOpen;
    bool isPreParserEnabled;
    bool isSettingsOpen;
    bool isSystemInfoOpen;
    int  pauseSelectedIndex;
    int  systemVolume; // 0 to 100 percent cache
    bool shouldCloseApp;
    bool shouldQuitOS;

    DebugStage  debugStage;
    DebugTarget debugActiveTarget;
    int debugSelectedIndex;
    int debugScrollLineOffset; // For scrolling inside our data textbox
} KernelState;

typedef struct {
    int x;
    int y;
} MouseState;

// Central core system application identifier registers
typedef struct {
    char inputBuffer[MAX_SHELL_COMMAND_LEN]; // Stores the currently typed live characters row
    int  cursorPosition;                     // Current character write index position inside the buffer
    int  blinkCounter;                       // Timer tick counter animating the flashing underscore cursor
    bool isCursorVisible;                    // Flashing toggle state switcher for rendering
    
    float promptX;                           // Screen position X anchor for the active command line
    float promptY;                           // Screen position Y anchor for the active command line
} ShellState;

typedef struct {
    unsigned char *spriteRAM;  // Pointer to 128x128 graphics RAM
    unsigned char *mapRAM;     // Pointer to 32x32 map RAM
    char *luaRAM;              // Pointer to raw active Lua text segment storage

    int spriteRAMSize;         // Max bounds (16384)
    int spriteRAMIndex;        // Live parsing tracker counter
    int spriteAtlasColumns;    // Active sheet stride (e.g. 128)
    int spriteAtlasRows;
    
    int mapRAMSize;            // Max bounds (1024)
    int mapRAMIndex;           // Live parsing tracker counter
    int mapColumns;
    int mapRows;

    int luaRAMSize;            // Maximum allocated buffer capacity bounds
    int luaRAMIndex;           // Live parsing string cursor index tracking positions

    char currentFilePath[MAX_CARTRIDGE_PATH_LEN];  // Encapsulates the active loaded file source path globally across all modules!
} CartridgeRAM;

// Storage capsule holding text metadata extracted from the loaded game file
typedef struct {
    char name[64];
    char author[64];
    char version[16];
    char license[32];
    char mode[15]; // Stores "pico8" or "picoray" Safely
    bool spriteZeroTransparent;
} CartridgeMeta;

typedef struct {
    bool isOpen;
    FileDialogMode mode;                   // Distinguishes between OPEN and SAVE states
    char files[MAX_FILE_DIALOG_FILES][64]; // Found files cache list (filenames up to 63 chars)
    int  fileCount;
    int  selectedIndex;
    int  scrollOffset;
    char extensionFilter[16];
    char inputTextBoxBuffer[64];       // Input text field buffer for typing new filenames during SAVE cycles
    int  inputTextLength;
    void (*onFileAction)(const char *filePath); // Unified dynamic callback trigger
} FileDialogState;

typedef struct {
    const char *name;
    const char *author;
    const char *version;
    const char *license;
    unsigned int appId;  // Dynamic ray cartridges generate a clean hashed ID from metadata. System apps use hardcoded values (1 = Shell, 2 = SpriteEdit, etc.)
    int  iconId;
    int  width;
    int  height;
    bool hasMenuBar;
} AppConfig;

typedef struct {
    AppConfig (*GetConfig)(void);
    void (*Init)(void);
    void (*RegisterMenus)(void); 
    void (*Update)(MouseState mousePos);
    void (*Draw)(void);
    void (*Cleanup)(void);
} AppInterface;

// Represents a single self-contained character asset tied directly to its Unicode identity
typedef struct {
    unsigned int  codePoint;  // The absolute Unicode ID (e.g., 'á' = 0x00E1, 'あ' = 0x3042)
    unsigned char bytes[12];  // FIX: Clean fixed buffer supporting standard 6px up to large 12px heights!
} GlyphMapping;

typedef enum {
    GLYPHMAP_ID_PICORAY_ASCII = 0,
    GLYPHMAP_ID_PICORAY_UTF8_HU,
    GLYPHMAP_ID_PICO8_ASCII,
    GLYPHMAP_ID_PICO8_EXTENDED,
    GLYPHMAP_ID_COUNT
} GlyphMapId;

typedef struct {
    bool isPrintable[MAX_FONT_CACHE_CODEPOINTS]; // Direct O(1) byte lookup for printability
    bool isAccented[MAX_FONT_CACHE_CODEPOINTS];  // Direct O(1) byte lookup for accent-shift tracking
} FontPreloadCache;

typedef struct {
    const char *iconName;
    int iconWidth;
    int iconHeight;
    const char data[6][7];  // The raw 6x6 pixel grid string matrix (+1 character for null terminator)
} IconRegistry;

typedef struct {
    const char *fontName;  // Human readable name
    int charWidth;         // Max width of a glyph
    int charHeight;        // Max uniform height of ALL glyph slots in memory
    int printableWidth;    // Target spacing width for standard readable ASCII

    int accentHeight;      // Fixed negative Y offset for rendering accents
    int lineGap;           // Fixed padding space added between text rows
    int charGap;           // Fixed horizontal spacing padding added between letter

    // Multi-table sharding matrix pointers
    const GlyphMapping *glyphTables[MAX_GLYPH_TABLES_PER_FONT];
    int glyphCounts[MAX_GLYPH_TABLES_PER_FONT];
    int totalTableCount;
} FontRegistry;

typedef struct {
    const char *paletteName;
    int colorCount;
    Color colors[32];
} PaletteRegistry;

typedef struct {
    const char *themeName;
    int topBarBg, topBarText;
    int topBarHoverBg, topBarHoverText;
    int menuCardBg, menuCardBorder, menuCardText;
    int menuWindowBg,      menuWindowText;
    int menuWindowHoverBg, menuWindowHoverText;
    int buttonBg, buttonHover, buttonPressed, buttonIcon, buttonText;
    int windowBg, windowBorder;
} ThemeRegistry;

typedef struct {
    const char *text;
    void (*onClick)(void); 
    int iconId;     
} MenuOption;

typedef struct {
    const char *title;
    int optionCount;
    MenuOption options[8]; // Fixed maximum 8 items per slot
    int calculatedX;       // Cached runtime offset geometry
    int calculatedWidth;   // Cached runtime click surface bounds
} Menu;

typedef struct {
    Menu systemMenus[MAX_SYSTEM_MENUS];
    int  totalActiveMenus;
    int  activeMenuIndex;
    bool isMenuBarVisible;
} MenuSystem;

typedef struct {
    int x, y, width, height;
    const char *text;
    int  iconId;          
    bool isHovered, isPressed;
    void (*onClick)(void);  
} Button;

extern const IconRegistry    iconData[ICON_COUNT];
extern const FontRegistry    fontData[FONT_COUNT];
extern const PaletteRegistry paletteData[PALETTE_COUNT];
extern const ThemeRegistry   themeData[THEME_COUNT];

// ap_sprite_editor.c
void* PR_GetSpriteEditorState(void);

// framework.c
// Central Subsystem Access Gateways
AudioSystem*     PR_GetAudioSystem(void);
GraphicsSystem*  PR_GetGraphicsSystem(void);
KernelState*     PR_GetKernelState(void);
CartridgeRAM*    PR_GetCartridgeRAM(void);
MenuSystem*      PR_GetMenuSystem(void);
FileDialogState* PR_GetFileDialogSatte(void);

AppConfig        PR_GetActiveAppConfig(void);

const CartridgeRAM* PR_GetDefaultCartridgeRAM(void);

void  PR_Callback_CloseApp(void); 
void  PR_Callback_CloseAboutWindow(void);
void  PR_Callback_CloseSystemInfoWindow(void);
void  PR_DrawMenuSystem(Menu *menus, int menuCount, int activeMenuVar);
int   PR_GetActiveFontId(void);
int   PR_GetActivePaletteId(void); 
int   PR_GetActiveThemeId(void);
bool  PR_GetShouldQuitOS(void);
void  PR_PreRenderFontAtlas(void);
bool  PR_LoadGlyphPack(GlyphMapId glyphPackId);
const GlyphMapping* PR_GetActiveGlyphPackTable(int *glyphCount);
void  PR_PreRenderIconAtlas(void);
void  PR_ResetColorTransparency(void);
void  PR_SetActiveFontId(int fontId);
void  PR_ToggleDebugSystem(void);
void  PR_TogglePauseMenu(void);
void  PR_UpdateMenuSystem(Menu *menus, int menuCount, int *activeMenuVar, MouseState mousePos);

bool  Update_PauseMenu(MouseState mousePos);
bool  Update_DebugWindow(MouseState mousePos);
bool  Update_FileDialogWindow(MouseState mousePos);
bool  Update_GlobalMenuBar(MouseState mousePos);

void  SwitchApp(AppInterface newApp);

void  Framework_Init(void);
void  Framework_Update(void);
void  Framework_Draw(void);
void  Framework_Cleanup(void);

// framework-debug.c
// Declare full telemetry inspector hook pipeline
void PR_ToggleDebugSystem(void);
void PR_UpdateDebugSystem(void);
void PR_DrawDebugSystem(void);

// pr.c
void  PR_Blit(const unsigned char *source, int srcW, int srcH, int destX, int destY, int colorId);
void  PR_Circ(float centerX, float centerY, float radius, int colorId);
void  PR_CircFill(float centerX, float centerY, float radius, int colorId);
void  PR_Cls(int colorId);
void  PR_DrawIcon(int iconId, float startX, float startY, int colorId);
void  PR_Line(float x1, float y1, float x2, float y2, int colorId);
void  PR_Oval(float centerX, float centerY, float radiusX, float radiusY, int colorId);
void  PR_OvalFill(float centerX, float centerY, float radiusX, float radiusY, int colorId);
void  PR_Print(const char *text, float startX, float startY, int colorId);  // Existing standard baseline printer
void  PR_PrintPro(int fontId, const char *text, float startX, float startY, int colorId);  // Allows developers to freely mix and render unlimited font faces simultaneously in a single frame pass!
int   PR_PGet(int x, int y);
void  PR_PSet(float x, float y, int colorId);
void  PR_Rect(int x, int y, int width, int height, int colorId);
void  PR_RectFill(float x, float y, float w, float h, int colorId);
int   PR_SGet(int sheetX, int sheetY);
void  PR_SSet(int sheetX, int sheetY, int colorId);
void  PR_Spr(const unsigned char *customBytes, int w, int h, float x, float y, int colorId);
void  PR_UpdateVRAM(int x, int y, int colorId);

// pr-app.c
unsigned int PR_GenerateAppHash(const char *title, const char *author);

// pr-audio.c
void PR_PlayMusic(const char *fileName);
void PR_StopMusic(void);
void PR_PlaySFX(const char *fileName);

// pr-cartridge.c
CartridgeMeta* PR_GetCartridgeMeta(void);
void  PR_LoadPluginCartridge(void);
bool PR_LoadRayCartridge(const char *filePath, CartridgeRAM *cart);
 
// pr-color.c
Color PR_GetColor(int colorId);
char  GetHexCharFromColor(Color c);
int   GetPicoColorFromColor(Color c);

// pr-debug.c
void  PR_Debug_DumpVRAM(void);
void  PR_PushKernelWarning(const char *message);
int   PR_GetKernelWarnings(const char **outLines, int maxLines);

// pr-font.c
void PR_InitFontCache(void);
bool PR_IsCharPrintable(int fontId, unsigned int codepoint);
bool PR_IsCharAccented(int fontId, unsigned int codepoint);
const unsigned char* PR_GetFontGlyph(int fontId, unsigned int codepoint);
int PR_GetStringWidthByPixel(int fontId, const char *text);

// pr-lua.c
void  PR_Lua_LoadAPI(lua_State *L);
void  PR_Lua_LoadPicoRayAPI(lua_State *L);
void  PR_Lua_LoadPico8CompatibilityAPI(lua_State *L);
void  PR_Lua_RegisterAudioAPI(lua_State *L);
char* PR_PreParseLuaSyntax(const char* sourceCode);

// pr-mouse.c
MouseState PR_GetMousePosition(void);

// pr-p8-compat.c
void  P8_Spr(int spriteId, float destX, float destY, float w, float h, bool flipX, bool flipY);

// pr-ui.c
Button PR_CreateButton(int x, int y, int w, int h, const char *text, int iconId, void (*callback)(void));
bool   PR_UpdateButton(Button *btn, MouseState mousePos);
void   PR_DrawButton(Button *btn);

// pr-ui-file-dialog.c
bool PR_IsFileDialogOpen(void);
void PR_OpenFileDialog(const char *folder, const char *extension, void (*callback)(const char *filePath));
void PR_SaveFileDialog(const char *folder, const char *extension, void (*callback)(const char *filePath));
void PR_UpdateFileDialog(MouseState mousePos);
void PR_DrawFileDialog(void);

// pr-ui-menu.c
Menu  PR_CreateMenu(const char *title);
void  PR_AddMenuItem(Menu *menu, const char *text, int iconId, void (*callback)(void));
void  PR_RegisterApplicationMenu(Menu appMenu);
Menu* PR_GetRegisteredMenu(const char *title);

// pr-ui-window.c
void  PR_DrawDebugWindow(void);
void  PR_DrawPauseWindow(void);
void  PR_DrawAboutWindow(void);
void  PR_DrawSystemInfoWindow(void);

// pr-ui-theme.c

// lua/api-lua.c
int   PR_Lua_Btn(lua_State *L);
int   PR_Lua_Btnp(lua_State *L);
int   PR_Lua_Cls(lua_State *L);
int   PR_Lua_ExitOS(lua_State *L);
int   PR_Lua_MusicPlay(lua_State *L);
int   PR_Lua_MusicStop(lua_State *L);
int   PR_Lua_PAlt(lua_State *L);
int   PR_Lua_PGet(lua_State *L);
int   PR_Lua_PSet(lua_State *L);
int   PR_Lua_Print(lua_State *L);
int   PR_Lua_PrintPro(lua_State *L);
int   PR_Lua_Rect(lua_State *L);
int   PR_Lua_RectFill(lua_State *L);
int   PR_Lua_SetFont(lua_State *L);
int   PR_Lua_SFXPlay(lua_State *L);
int   PR_Lua_SGet(lua_State *L);
int   PR_Lua_SSet(lua_State *L);

// lua/api-lua-pico8.c
int   P8_Lua_MGet(lua_State *L);
int   P8_Lua_MSet(lua_State *L);
int   P8_Lua_Spr(lua_State *L);

// lua/api-lua-picoray.c
int   PR_Lua_MGet(lua_State *L);
int   PR_Lua_MSet(lua_State *L);
int   PR_Lua_Spr(lua_State *L);

#endif // FRAMEWORK_H
