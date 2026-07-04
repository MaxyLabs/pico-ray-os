#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "framework.h"

// Forward declare application instances
extern AppInterface Home_App;

// Base hardware default parameters mapped out cleanly
static const GraphicsSystem defaultGraphics = {
    .virtualWidth          = 128,
    .virtualHeight         = 128,
    .screenScale           = 5,
    .currentFontId         = FONT_PICORAY,
    .currentPaletteId      = PALETTE_PICO8,
    .currentThemeId        = THEME_LIGHT,
    .systemFontId          = FONT_PICORAY,
    .isColorTransparent[0] = true
};

// Pico8 compatible default settings
static const CartridgeRAM defaultCartridgeRAM = {
    .spriteRAM           = NULL,
    .spriteRAMSize       = 0,
    .spriteRAMIndex      = 0,
    .spriteAtlasColumns  = 128,
    .spriteAtlasRows     = 128,
    .mapRAM              = NULL,
    .mapRAMSize          = 0,
    .mapRAMIndex         = 0,
    .mapColumns          = 128,
    .mapRows             = 32
};

static FileDialogState fileDialogState = {
    .isOpen            = false,
    .mode              = FILE_DIALOG_OPEN,
    .fileCount         = 0,
    .selectedIndex     = -1,
    .scrollOffset      = 0,
    .inputTextLength   = 0
};

// Isolated persistent subsystem instances inside core RAM (Sealed inside this translation unit safely)
static Color *presentBuffer     = NULL;
static int    presentBufferSize = 0;

static AudioSystem    audioSystem;
static GraphicsSystem graphicsSystem;
static KernelState    kernelState;
static MenuSystem     menuSystem;
static CartridgeRAM   cartridgeRAM;
static CartridgeMeta  cartridgeMeta;
static AppInterface   activeApp;
static char appSwitchStatusMessage[96];
static int appSwitchStatusFrames = 0;

#define KERNEL_WARNING_RING_CAPACITY 8
static char kernelWarningRing[KERNEL_WARNING_RING_CAPACITY][96];
static int kernelWarningRingStart = 0;
static int kernelWarningRingCount = 0;

void PR_PushKernelWarning(const char *message) {
    if (message == NULL || message[0] == '\0') return;

    int writeIndex;
    if (kernelWarningRingCount < KERNEL_WARNING_RING_CAPACITY) {
        writeIndex = (kernelWarningRingStart + kernelWarningRingCount) % KERNEL_WARNING_RING_CAPACITY;
        kernelWarningRingCount++;
    } else {
        writeIndex = kernelWarningRingStart;
        kernelWarningRingStart = (kernelWarningRingStart + 1) % KERNEL_WARNING_RING_CAPACITY;
    }

    snprintf(kernelWarningRing[writeIndex], sizeof(kernelWarningRing[writeIndex]), "%s", message);
}

int PR_GetKernelWarnings(const char **outLines, int maxLines) {
    if (outLines == NULL || maxLines <= 0 || kernelWarningRingCount <= 0) return 0;

    int count = kernelWarningRingCount;
    if (count > maxLines) count = maxLines;

    int startOffset = kernelWarningRingCount - count;
    for (int i = 0; i < count; i++) {
        int idx = (kernelWarningRingStart + startOffset + i) % KERNEL_WARNING_RING_CAPACITY;
        outLines[i] = kernelWarningRing[idx];
    }

    return count;
}

static void SetAppSwitchStatusMessage(const char *message) {
    if (message == NULL || message[0] == '\0') {
        appSwitchStatusMessage[0] = '\0';
        appSwitchStatusFrames = 0;
        return;
    }

    snprintf(appSwitchStatusMessage, sizeof(appSwitchStatusMessage), "%s", message);
    appSwitchStatusFrames = 240;
}

// --- Central Subsystem Core Access Gateways ---
const CartridgeRAM*   PR_GetDefaultCartridgeRAM(void) { return &defaultCartridgeRAM; }

AudioSystem*     PR_GetAudioSystem(void)     { return &audioSystem; }
GraphicsSystem*  PR_GetGraphicsSystem(void)  { return &graphicsSystem; }
KernelState*     PR_GetKernelState(void)     { return &kernelState; }
CartridgeRAM*    PR_GetCartridgeRAM(void)    { return &cartridgeRAM; }
CartridgeMeta*   PR_GetCartridgeMeta(void)   { return &cartridgeMeta; }
MenuSystem*      PR_GetMenuSystem(void)      { return &menuSystem; }
FileDialogState* PR_GetFileDialogSatte(void) { return &fileDialogState; }
AppConfig        PR_GetActiveAppConfig(void) { return activeApp.GetConfig(); }

bool PR_GetShouldQuitOS(void)    { return kernelState.shouldQuitOS; }
int  PR_GetActivePaletteId(void) { return graphicsSystem.currentPaletteId; }
int  PR_GetActiveFontId(void)    { return graphicsSystem.currentFontId; }
int  PR_GetActiveThemeId(void)   { return graphicsSystem.currentThemeId; }

// Safety global query tracking validation status properties cleanly
bool PR_IsFileDialogOpen(void) { return fileDialogState.isOpen; }

// Internal Core Callback Functions for System Actions
static void Callback_OpenAbout(void)    { kernelState.isAboutOpen = true; }
static void Callback_OpenSysInfo(void)  { kernelState.isSystemInfoOpen = true; }
static void Callback_OpenSettings(void) { kernelState.isSettingsOpen = true; }
static void Callback_QuitOS(void)       { kernelState.shouldQuitOS = true; }

// --- Inside src/framework/framework.c ---

void PR_PreRenderFontAtlas(void) {
    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) return;

    FontRegistry font = fontData[activeFontId];
    if (font.totalTableCount <= 0) return;

    // FIX 3: COMPUTE TOTAL GLYPHS ACROSS ALL COMBINED SUB-TABLES
    // We sum up the individual lengths of each registered active dictionary segment
    int totalGlyphsInAtlas = 0;
    for (int t = 0; t < font.totalTableCount; t++) {
        totalGlyphsInAtlas += font.glyphCounts[t];
    }
    if (totalGlyphsInAtlas <= 0) return;

    int atlasW = totalGlyphsInAtlas * font.charWidth;
    int atlasH = font.charHeight;

    UnloadImage(graphicsSystem.fontAtlas); 
    graphicsSystem.fontAtlas = GenImageColor(atlasW, atlasH, BLANK); 

    int currentGlyphIndexGlobal = 0;

    // Outer loop scales through each registered sub-table layer
    for (int t = 0; t < font.totalTableCount; t++) {
        const GlyphMapping *currentTable = font.glyphTables[t];
        int currentCount = font.glyphCounts[t];
        
        if (currentTable == NULL || currentCount <= 0) continue;

        for (int i = 0; i < currentCount; i++) {
            int glyphOffsetX = currentGlyphIndexGlobal * font.charWidth;
            
            // Re-route token queries directly through our smart baseline-aligned lookup engine
            // This natively applies the 2px downward padding offset inside the atlas texture automatically!
            const unsigned char *glyphBytes = PR_GetFontGlyph(activeFontId, currentTable[i].codePoint);
            
            if (glyphBytes != NULL) {
                for (int y = 0; y < font.charHeight; y++) {
                    unsigned char row = glyphBytes[y];
                    unsigned char mask = 0x01; 
                    
                    for (int x = 0; x < font.charWidth; x++) {
                        if ((row & mask) > 0) {
                            ImageDrawPixel(&graphicsSystem.fontAtlas, glyphOffsetX + (font.charWidth - 1 - x), y, WHITE);
                        }
                        mask <<= 1;
                    }
                }
            }
            currentGlyphIndexGlobal++; // Advance global atlas horizontal tile column counter
        }
    }
    printf("PICO-RAY OS | FONTS | Built layered multi-table texture atlas holding %d glyphs.\n", totalGlyphsInAtlas);
}


void PR_PreRenderIconAtlas(void) {
    // Width: 6 icons * 6px = 36px. Height: 6px.
    int atlasW = ICON_COUNT * 6;
    int atlasH = 6;

    UnloadImage(graphicsSystem.iconAtlas); // Safety flush
    graphicsSystem.iconAtlas = GenImageColor(atlasW, atlasH, BLANK); // 100% transparent start

    for (int i = 0; i < ICON_COUNT; i++) {
        int iconOffsetX = i * 6;
        IconRegistry icon = iconData[i];

        for (int y = 0; y < icon.iconHeight; y++) {
            for (int x = 0; x < icon.iconWidth; x++) {
                char marker = icon.data[y][x];
                if (marker == 'X' || marker == '0') {
                    // Paint as solid WHITE to create a perfect tintable stencil mask sheet
                    ImageDrawPixel(&graphicsSystem.iconAtlas, iconOffsetX + x, y, WHITE);
                }
            }
        }
    }
}

void PR_AddSystemMenuDefaults(void) {
    // Fetch live target application configuration metrics
    AppConfig config = activeApp.GetConfig();

    Menu appleMenu = PR_CreateMenu("o");
    PR_AddMenuItem(&appleMenu, "About App",   ICON_INFO, Callback_OpenAbout);
    PR_AddMenuItem(&appleMenu, "System Info", ICON_EMPTY, Callback_OpenSysInfo);
    PR_AddMenuItem(&appleMenu, "Settings",    ICON_TOOLS, Callback_OpenSettings);
    PR_AddMenuItem(&appleMenu, "Quit OS",     ICON_QUIT,  Callback_QuitOS);
    PR_RegisterApplicationMenu(appleMenu);

    // Create a completely EMPTY File menu container.
    // The application will append its own triggers first, and the kernel will lock the bottom exit row later.
    Menu fileMenu = PR_CreateMenu("File");
    PR_RegisterApplicationMenu(fileMenu);
}

// Clears layout data pools and executes dynamic spacing bounding checks matching font typography rules
// --- Inside framework/modules/pr-ui-menu.c ---

// Clears layout data pools and executes dynamic spacing bounding checks matching font typography rules
void PR_RebuildMenuBar(void) {
    MenuSystem *menuSys = PR_GetMenuSystem();
    if (menuSys == NULL) return;

    // 1. Flush and completely clear out previous active navigation data memory segments
    memset(menuSys, 0, sizeof(MenuSystem));

    // Fetch live target application metrics via our clean API proxy gate
    AppConfig config = PR_GetActiveAppConfig();

    // FIX: Set the visibility flag based on config, but DO NOT abort the generation!
    // We always build the menu tree structures in RAM so they are ready for the 'M' toggle key!
    menuSys->isMenuBarVisible = config.hasMenuBar;

    // 2. Inject standard systemic fallback tab structures (Apple tab and clean File tab)
    PR_AddSystemMenuDefaults();

    // 3. TRIGGER APP-SPECIFIC DROP INJECTIONS (Edit, Tools, custom layers, etc.)
    extern AppInterface activeApp; // Linked smoothly to the global scheduler instance channel
    if (kernelState.isAppLoaded && activeApp.RegisterMenus != NULL) {
        activeApp.RegisterMenus();
    }

    // 4. MACOS STYLE FALLBACK: Dynamically force the exit close shortcut row to sit strictly at the bottom
    if (strcmp(config.name, "Home") != 0 && strcmp(config.name, "PICO-RAY Home") != 0) {
        Menu *fileMenuPtr = PR_GetRegisteredMenu("File");
        
        if (fileMenuPtr != NULL && fileMenuPtr->optionCount < 8) {
            int bottomSlot = fileMenuPtr->optionCount;
            
            fileMenuPtr->options[bottomSlot].text = "Close App";
            fileMenuPtr->options[bottomSlot].iconId = ICON_CLOSE;
            fileMenuPtr->options[bottomSlot].onClick = PR_Callback_CloseApp;
            fileMenuPtr->optionCount++;
        }
    }

    // 5. RUN HORIZONTAL GEOMETRY BOXES SPATIAL CALCULATION ENGINE
    int currentX = 4;
    int activeFontId = PR_GetActiveFontId();

    for (int i = 0; i < menuSys->totalActiveMenus; i++) {
        // Skip spatial computation layout logic for empty menu headers!
        if (menuSys->systemMenus[i].optionCount == 0) {
            menuSys->systemMenus[i].calculatedX = 0;
            menuSys->systemMenus[i].calculatedWidth = 0;
            continue;
        }

        menuSys->systemMenus[i].calculatedX = currentX;
        int textLength = (int)strlen(menuSys->systemMenus[i].title);
    
        // Clean single-character string parsing matching the Apple logo 'o' placeholder token safely
        if (i == 0 && menuSys->systemMenus[i].title[0] == 'o' && strlen(menuSys->systemMenus[i].title) == 1) {
            menuSys->systemMenus[i].calculatedWidth = 10; 
        } else {
            // Track widths proportionally using our clean font register metrics rules!
            menuSys->systemMenus[i].calculatedWidth = (textLength * fontData[activeFontId].printableWidth) + 6; 
        }

        currentX += menuSys->systemMenus[i].calculatedWidth + 6; 
    }

    menuSys->activeMenuIndex = 0; // Lock structural index safely to ground state
}

void PR_ResetColorTransparency(void) {
    // Inside your graphics initialization function
    for (int i = 0; i < 32; i++) {
        graphicsSystem.isColorTransparent[i] = (i == 0); // Csak a 0 indexű fekete lesz igaz (true)
    }
}

// DYNAMIC FONT SWITCHER WITH AUTOMATIC GPU ATLAS RE-BUILD PIPELINE
void PR_SetActiveFontId(int fontId) {
    if (fontId < 0 || fontId >= FONT_COUNT) return;
    
    graphicsSystem.currentFontId = fontId;
    
    // Dynamic Hot-Reload: Instantly rebuild the texture atlas structure for the new layout!
    PR_PreRenderFontAtlas();
    
    printf("PICO-RAY OS | FONTS | Active typeface changed to ID: %d (%s)\n", 
            fontId, fontData[fontId].fontName);
}

void SwitchApp(AppInterface newApp) {
    AppConfig newConfig = newApp.GetConfig();

    int targetWidth = newConfig.width;
    int targetHeight = newConfig.height;
    int requiredPixels = targetWidth * targetHeight;

    // System workspace utilities (Shell, SpriteEdit) live in the tight ID range 1 to SYS_APP_COUNT.
    bool isTargetSystemApp = (newConfig.appId > 0 && newConfig.appId < SYS_APP_COUNT);

    // Dynamic sizing queries fetched from default constants registers
    int spriteCols = defaultCartridgeRAM.spriteAtlasColumns;
    int spriteRows = defaultCartridgeRAM.spriteAtlasRows;
    int spriteSize = spriteCols * spriteRows;

    int mapCols = defaultCartridgeRAM.mapColumns;
    int mapRows = defaultCartridgeRAM.mapRows;
    int mapSize = mapCols * mapRows;

    // We initialize pointer holders to NULL by default
    unsigned char *newSpriteRAM = NULL;
    unsigned char *newMapRAM = NULL;   

    // Only allocate distinct memory channels if loading an external game cartridge (high hashed ID)
    if (!isTargetSystemApp) {
        printf("PICO-RAY OS | KERNEL | External game cart selected (ID: 0x%08X). Performing RAM format reset.\n", newConfig.appId);
        
        newSpriteRAM = (unsigned char *)malloc((size_t)spriteSize);
        if (newSpriteRAM == NULL) {
            printf("PICO-RAY OS | KERNEL | ERROR: Failed to allocate sprite RAM while switching app.\n");
            SetAppSwitchStatusMessage("Switch failed: sprite RAM alloc");
            PR_PushKernelWarning("Switch failed: sprite RAM alloc");
            return;
        }
        memset(newSpriteRAM, 0, (size_t)spriteSize);

        newMapRAM = (unsigned char *)malloc((size_t)mapSize);
        if (newMapRAM == NULL) {
            free(newSpriteRAM);
            printf("PICO-RAY OS | KERNEL | ERROR: Failed to allocate map RAM while switching app.\n");
            SetAppSwitchStatusMessage("Switch failed: map RAM alloc");
            PR_PushKernelWarning("Switch failed: map RAM alloc");
            return;
        }
        memset(newMapRAM, 0, (size_t)mapSize);
    } else {
        printf("PICO-RAY OS | KERNEL | Switched to Workspace Tool (ID: %d). CartridgeRAM protected and locked.\n", newConfig.appId);
    }

    // Allocate the Virtual Screen Grayscale VRAM Buffer (Always needed to reset resolutions layers)
    void *newVirtualVRAMData = malloc((size_t)requiredPixels);
    if (newVirtualVRAMData == NULL) {
        if (newMapRAM) free(newMapRAM);
        if (newSpriteRAM) free(newSpriteRAM);
        printf("PICO-RAY OS | KERNEL | ERROR: Failed to allocate virtual VRAM while switching app.\n");
        SetAppSwitchStatusMessage("Switch failed: VRAM alloc");
        PR_PushKernelWarning("Switch failed: VRAM alloc");
        return;
    }
    memset(newVirtualVRAMData, 0, (size_t)requiredPixels);

    // Resize or grow the presentation RGBA monitor pixel color buffer strip arrays
    Color *newPresentBuffer = presentBuffer;
    int newPresentBufferSize = presentBufferSize;
    if (requiredPixels > presentBufferSize) {
        newPresentBuffer = (Color *)realloc(presentBuffer, (size_t)requiredPixels * sizeof(Color));
        if (newPresentBuffer == NULL) {
            free(newVirtualVRAMData);
            if (newMapRAM) free(newMapRAM);
            if (newSpriteRAM) free(newSpriteRAM);
            printf("PICO-RAY OS | KERNEL | ERROR: Failed to grow present buffer while switching app.\n");
            SetAppSwitchStatusMessage("Switch failed: present buffer alloc");
            PR_PushKernelWarning("Switch failed: present buffer alloc");
            return;
        }
        newPresentBufferSize = requiredPixels;
    }

    // Call previous application lifecycle cleanup handlers
    if (kernelState.isAppLoaded && activeApp.Cleanup != NULL) { 
        activeApp.Cleanup(); 
    }

    // Bind the new operational runtime properties
    activeApp = newApp;
    kernelState.isAppLoaded = true;

    graphicsSystem.virtualWidth = targetWidth;
    graphicsSystem.virtualHeight = targetHeight;

    PR_ResetColorTransparency();

    // We strictly avoid freeing the persistent boot-time structures when cycling between workspace tools!
    if (!isTargetSystemApp) {
        if (cartridgeRAM.spriteRAM != NULL) free(cartridgeRAM.spriteRAM);
        cartridgeRAM.spriteRAM = newSpriteRAM;
        cartridgeRAM.spriteAtlasColumns = spriteCols;
        cartridgeRAM.spriteAtlasRows = spriteRows;
        cartridgeRAM.spriteRAMSize = spriteSize;

        if (cartridgeRAM.mapRAM != NULL) free(cartridgeRAM.mapRAM);
        cartridgeRAM.mapRAM = newMapRAM;
        cartridgeRAM.mapColumns = mapCols;
        cartridgeRAM.mapRows = mapRows;
        cartridgeRAM.mapRAMSize = mapSize;
    } else {
        // Safe resets for internal cursors to guarantee file loaders re-sync from index 0 safely
        cartridgeRAM.spriteRAMIndex = 0;
        cartridgeRAM.mapRAMIndex = 0;
    }

    // Update Virtual GrayScale Core VRAM Image Properties
    UnloadImage(graphicsSystem.virtualVRAM);
    graphicsSystem.virtualVRAM.data = newVirtualVRAMData;
    graphicsSystem.virtualVRAM.width = targetWidth;
    graphicsSystem.virtualVRAM.height = targetHeight;
    graphicsSystem.virtualVRAM.mipmaps = 1;
    graphicsSystem.virtualVRAM.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

    presentBuffer = newPresentBuffer;
    presentBufferSize = newPresentBufferSize;

    // Reset and rescale the GPU display rendering texture filters
    UnloadTexture(graphicsSystem.virtualScreenTexture);
    Image rgbaImage = GenImageColor(targetWidth, targetHeight, BLACK);
    graphicsSystem.virtualScreenTexture = LoadTextureFromImage(rgbaImage);
    UnloadImage(rgbaImage);
    SetTextureFilter(graphicsSystem.virtualScreenTexture, TEXTURE_FILTER_POINT);

    // Dynamically adjust the OS physical desktop canvas window scale geometry bounds
    SetWindowSize(targetWidth * graphicsSystem.screenScale, targetHeight * graphicsSystem.screenScale);

    // Bootstrap the upcoming application's local initializer loop
    if (activeApp.Init != NULL) { 
        activeApp.Init(); 
    }

    // Refresh dynamic core typography and icon layouts atlas matrices inside memory
    PR_PreRenderFontAtlas();
    PR_PreRenderIconAtlas();
    PR_RebuildMenuBar();

    SetAppSwitchStatusMessage(NULL);
    kernelState.shouldCloseApp = false;
}

void PR_Callback_CloseAboutWindow(void) { kernelState.isAboutOpen = false; }
void PR_Callback_CloseSystemInfoWindow(void) { kernelState.isSystemInfoOpen = false; }
void PR_Callback_CloseApp(void) {
    printf("PICO-RAY OS | KERNEL: Ejecting active application. Cleaning audio registers...\n");
    
    // Forcefully kill any active streaming track background cycles right before unloading the workspace app
    PR_StopMusic();

    // Your existing app closing and state flags updates continue below safely...
    kernelState.isPauseMenuOpen = false;
    kernelState.shouldCloseApp = true;
}

void PR_DrawPauseWindow(void) {
    // Renders on top of everything inside the active virtualVRAM frame matrix
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    ThemeRegistry theme = themeData[PR_GetActiveThemeId()];
    // A. TRANSLUCENT OVERLAY FILTER SHADE SCREEN
    // We use our intermediate mask array to darken the underlying active game frame layout
    PR_RectFill(0, 0, 128, 128, P8_EXT_BLACK); // Shade backdrop entirely
    // B. RENDER MAIN WINDOW PLATE (Centred 96x80 window panel box)
    int winX = 16; int winY = 24;
    int winW = 96; int winH = 80;
    
    PR_RectFill(winX, winY, winW, winH, P8_LIGHT_GREY);
    PR_Rect(winX, winY, winW, winH, P8_DARK_GREY);
    
    // Header plate row accent banner
    PR_RectFill(winX + 1, winY + 1, winW - 2, 9, P8_DARK_BLUE);
    PR_PrintPro(gfx->systemFontId, "SYSTEM PAUSED", winX + 12, winY + 2, P8_WHITE);
    // C. RENDER INTERACTIVE MENU SELECTION ROWS
    const char *menuStrings[] = { "RESUME GAME", "RESET CART", "VOLUME", "RETURN HOME" };
    
    for (int i = 0; i < 4; i++) {
        int rowY = winY + 18 + (i * 12);
        int rowW = winW - 12;
        int rowX = winX + 6;
        // Live responsive text highlight tracking selectors hooks
        if (i == kernelState.pauseSelectedIndex) {
            // Active hover line selection row indicator box
            PR_RectFill(rowX, rowY - 1, rowW, 9, P8_BLUE);
            
            // Add a micro selection dot indicator symbol
            PR_PrintPro(gfx->systemFontId, ">", rowX + 2, rowY, P8_YELLOW);
            
            if (i == 2) {
                char volText[16]; sprintf(volText, "VOLUME: %d%%", kernelState.systemVolume);
                PR_PrintPro(gfx->systemFontId, volText, rowX + 10, rowY, P8_WHITE);
            } else {
                PR_PrintPro(gfx->systemFontId, menuStrings[i], rowX + 10, rowY, P8_WHITE);
            }
        } else {
            if (i == 2) {
                char volText[16]; sprintf(volText, "VOLUME: %d%%", kernelState.systemVolume);
                PR_PrintPro(gfx->systemFontId, volText, rowX + 10, rowY, P8_DARK_GREY);
            } else {
                PR_PrintPro(gfx->systemFontId, menuStrings[i], rowX + 10, rowY, P8_DARK_GREY);
            }
        }
    }
    
    // Micro branding line row layout footer decor strip
    PR_Line(winX + 6, winY + 72, winX + winW - 6, winY + 72, P8_DARK_GREY);
    PR_PrintPro(gfx->systemFontId, "PICO-RAY OS v0.2", winX + 14, winY + 74, P8_DARK_GREY);
}


// --- Framework --- Init / Update / Draw / Cleanup ---

void Framework_Init(void) {
    memset(&kernelState, 0, sizeof(KernelState));
    kernelState.isPreParserEnabled = true;

    //  KERNEL BOOT SEQUENCE & DATA-DRIVEN PRELOAD CACHE
    // This runs exactly ONCE at system startup!
    // It scans raw hardware table shards and builds the O(1) bool lookup tables.
    PR_InitFontCache();

    graphicsSystem = defaultGraphics;

    // We physically allocate the permanent CartridgeRAM buffers right here at boot-time!
    // This ensures that the spriteRAM and mapRAM pointers are NEVER NULL when system apps start.
    const CartridgeRAM *defaultCart = PR_GetDefaultCartridgeRAM();

    cartridgeRAM.spriteAtlasColumns = defaultCart->spriteAtlasColumns;
    cartridgeRAM.spriteAtlasRows    = defaultCart->spriteAtlasRows;
    cartridgeRAM.spriteRAMSize      = cartridgeRAM.spriteAtlasColumns * cartridgeRAM.spriteAtlasRows; // 16384 bytes
    cartridgeRAM.spriteRAM          = (unsigned char *)malloc((size_t)cartridgeRAM.spriteRAMSize);
    
    cartridgeRAM.mapColumns         = defaultCart->mapColumns;
    cartridgeRAM.mapRows            = defaultCart->mapRows;
    cartridgeRAM.mapRAMSize         = cartridgeRAM.mapColumns * cartridgeRAM.mapRows; // 1024 bytes
    cartridgeRAM.mapRAM             = (unsigned char *)malloc((size_t)cartridgeRAM.mapRAMSize);

    // Securely wipe the fresh desktop workspace matrices with clean zeros
    if (cartridgeRAM.spriteRAM != NULL) memset(cartridgeRAM.spriteRAM, 0, (size_t)cartridgeRAM.spriteRAMSize);
    if (cartridgeRAM.mapRAM != NULL)    memset(cartridgeRAM.mapRAM, 0, (size_t)cartridgeRAM.mapRAMSize);
    
    printf("PICO-RAY OS | KERNEL | Successfully initialized persistent hardware core Workspace Table RAM.\n");

    // Boot-time typography initialization
    PR_SetActiveFontId(FONT_PICORAY);

    InitWindow(graphicsSystem.virtualWidth * graphicsSystem.screenScale, graphicsSystem.virtualHeight * graphicsSystem.screenScale, "PICO-RAY Operating System");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // Clear and initialize our central audio structure capsule
    memset(&audioSystem, 0, sizeof(AudioSystem));
    audioSystem.isMusicActive = false;
    audioSystem.masterVolume = 1.0f;

    InitAudioDevice();
    if (IsAudioDeviceReady()) {
        printf("PICO-RAY OS | KERNEL: Sound card initialized cleanly via structured audio register.\n");
    } else {
        printf("PICO-RAY OS | KERNEL CRITICAL ERROR: Audio device failed to spin up!\n");
    }

    // Switch to Home App
    SwitchApp(Home_App);
}

// --- Inside src/framework/framework.c ---

bool Update_PauseMenu(MouseState mousePos) {
    // Persistent cache variable to store the volume level right before triggering MUTE
    static int cachedVolumeBeforeMute = 75; 

    // Disable the PauseMenu when the active app is: Home
    if (kernelState.isAppLoaded && strcmp(activeApp.GetConfig().name, "Home") != 0) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            kernelState.isPauseMenuOpen = !kernelState.isPauseMenuOpen;

            if (kernelState.isPauseMenuOpen) {
                kernelState.pauseSelectedIndex = 0; // Reset selector row anchor
            }
        }
    }

    if (kernelState.isPauseMenuOpen) {
        // Handle menu selection inputs row by row (Vertical navigation)
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            kernelState.pauseSelectedIndex--;
            if (kernelState.pauseSelectedIndex < 0) kernelState.pauseSelectedIndex = 3;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            kernelState.pauseSelectedIndex++;
            if (kernelState.pauseSelectedIndex > 3) kernelState.pauseSelectedIndex = 0;
        }

        // HORIZONTAL CONTROLLER INTERCEPTOR FOR VOLUME SLIDER ROW!
        if (kernelState.pauseSelectedIndex == 2) {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                kernelState.systemVolume -= 10;
                if (kernelState.systemVolume < 0) kernelState.systemVolume = 0; // Lock strict 0% floor limit
                printf("PICO-RAY OS | AUDIO: Master volume decreased down to %d%%\n", kernelState.systemVolume);
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                kernelState.systemVolume += 10;
                if (kernelState.systemVolume > 100) kernelState.systemVolume = 100; // Lock strict 100% ceiling cap
                printf("PICO-RAY OS | AUDIO: Master volume increased up to %d%%\n", kernelState.systemVolume);
            }
        }

        // Action activation pass via ENTER/SPACE confirmation keystrokes
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (kernelState.pauseSelectedIndex == 0) { // RESUME
                kernelState.isPauseMenuOpen = false;
            }
            else if (kernelState.pauseSelectedIndex == 1) { // RESET CART
                kernelState.isPauseMenuOpen = false;
                if (activeApp.Init != NULL) activeApp.Init(); // Re-bootstrap application life-cycle
            }
            else if (kernelState.pauseSelectedIndex == 2) { 
                // FIX: INSTANT TOGGLE MUTE / UNMUTE PIPELINE!
                // If the system is currently playing audio, mute it and cache the previous level.
                // If already muted, restore the precise percentage instantly!
                if (kernelState.systemVolume > 0) {
                    cachedVolumeBeforeMute = kernelState.systemVolume; // Lock current volume to cache
                    kernelState.systemVolume = 0;                      // Set instantly to pure silence
                    printf("PICO-RAY OS | AUDIO: Hardware MUTED [Muted from %d%%]\n", cachedVolumeBeforeMute);
                } else {
                    // Restore from cache, ensuring we don't unmute back into a 0% loop trap
                    kernelState.systemVolume = (cachedVolumeBeforeMute > 0) ? cachedVolumeBeforeMute : 75;
                    printf("PICO-RAY OS | AUDIO: Hardware UNMUTED straight back to %d%%\n", kernelState.systemVolume);
                }
            }
            else if (kernelState.pauseSelectedIndex == 3) { // CLOSE APP / RETURN HOME
                kernelState.isPauseMenuOpen = false;
                PR_Callback_CloseApp(); // Force return channel back to desktop space
            }
        }

        return false;  // Blocks input: Freezes further app logical updates
    } else {
        return true;
    }
}

bool Update_DebugWindow(MouseState mousePos) {
    if (IsKeyPressed(KEY_F4))  { PR_ToggleDebugSystem(); }
    // Pressing F12 instantly captures the active VRAM stream array state and dumps it into stdout terminal command line
    if (IsKeyPressed(KEY_F12)) { PR_Debug_DumpVRAM(); }

    if (kernelState.isDebugOpen) {
        PR_UpdateDebugSystem();
        return false; // Blocks input: Freezes further app logical updates
    } else {
        return true;
    }
}

bool Update_AboutWindow(MouseState mousePos) {
    if (kernelState.isAboutOpen) {
        return false;  // Blocks input: Freezes further app logical updates
    } else {
        return true;
    }
}

bool Update_FileDialogWindow(MouseState mousePos) {
    if (PR_IsFileDialogOpen()) {
        PR_UpdateFileDialog(mousePos);
        return false;  // Blocks input: Freezes further app logical updates
    } else {
        return true;
    }
}

bool Update_GlobalMenuBar(MouseState mousePos) {
    if (IsKeyPressed(KEY_M)) { menuSystem.isMenuBarVisible = !menuSystem.isMenuBarVisible; }

    if (menuSystem.isMenuBarVisible) {
        PR_UpdateMenuSystem(menuSystem.systemMenus, menuSystem.totalActiveMenus, &menuSystem.activeMenuIndex, mousePos);
    }

    if (menuSystem.activeMenuIndex == 0 ) {
        return true;
    } else {
        return false;  // Blocks input: Freezes further app logical updates
    }
}

void Framework_Update(void) {
    MouseState mousePos = PR_GetMousePosition();
    static bool allowNextStage;

    if (appSwitchStatusFrames > 0) {
        appSwitchStatusFrames--;
        if (appSwitchStatusFrames == 0) {
            appSwitchStatusMessage[0] = '\0';
        }
    }

    // STRUCTURED AUDIO STREAM MANAGER
    // We fetch our clean hardware register references via structure pointers!
    if (audioSystem.isMusicActive) {
        UpdateMusicStream(audioSystem.backgroundMusic);
        
        // Map our global systemVolume percentage (0-100) safely onto float volume ranges (0.0f - 1.0f)
        audioSystem.masterVolume = (float)PR_GetKernelState()->systemVolume / 100.0f;
        SetMusicVolume(audioSystem.backgroundMusic, audioSystem.masterVolume);
    }

    if (Update_PauseMenu(mousePos)) {
        if (Update_DebugWindow(mousePos)) {
            if (Update_AboutWindow(mousePos)) {
                if (Update_FileDialogWindow(mousePos)) {
                    if (Update_GlobalMenuBar(mousePos)) {
                        if (IsKeyPressed(KEY_P)) { graphicsSystem.currentPaletteId = (graphicsSystem.currentPaletteId + 1) % PALETTE_COUNT; }
                        if (kernelState.isAppLoaded && activeApp.Update != NULL) { activeApp.Update(mousePos); }
                        if (kernelState.shouldCloseApp) { SwitchApp(Home_App); }
                    }
                }
            }
        }
    }
}

void Framework_Draw(void) {
    AppConfig config = activeApp.GetConfig();
    GraphicsSystem *gfx = PR_GetGraphicsSystem();

    // CORE SOFTWARE RENDER PASS
    // Execute primary application graphics render cycle parsing state dependencies
    if (kernelState.isAppLoaded && activeApp.Draw != NULL) { 
        activeApp.Draw(); 
    }

    if (kernelState.isPauseMenuOpen)  { PR_DrawPauseWindow(); }
    // if (kernelState.isDebugOpen)     { PR_DrawDebugWindow(); }
    //if (dialogState.isOpen)          { PR_DrawFileDialog(); }
    if (PR_IsFileDialogOpen())        { PR_DrawFileDialog(); }
    if (kernelState.isAboutOpen)      { PR_DrawAboutWindow(); }
    if (kernelState.isSystemInfoOpen) { PR_DrawSystemInfoWindow(); }
    if (menuSystem.isMenuBarVisible)  { PR_DrawMenuSystem(menuSystem.systemMenus, menuSystem.totalActiveMenus, menuSystem.activeMenuIndex); }

    if (appSwitchStatusFrames > 0 && appSwitchStatusMessage[0] != '\0') {
        int bannerY = graphicsSystem.virtualHeight - 9;
        if (bannerY < 0) bannerY = 0;
        PR_RectFill(0, (float)bannerY, (float)graphicsSystem.virtualWidth, 9, P8_RED);
        PR_PrintPro(gfx->systemFontId, appSwitchStatusMessage, 2, (float)(bannerY + 1), P8_WHITE);
    }

    PR_DrawDebugSystem();    

    // 2. Convert palette indices to real RGBA colors before uploading to the GPU texture.
    if (presentBuffer != NULL && graphicsSystem.virtualVRAM.data != NULL) {
        int pixelCount = graphicsSystem.virtualWidth * graphicsSystem.virtualHeight;
        if (presentBufferSize >= pixelCount) {
            unsigned char *indexBytes = (unsigned char *)graphicsSystem.virtualVRAM.data;
            for (int i = 0; i < pixelCount; i++) {
                presentBuffer[i] = PR_GetColor(indexBytes[i]);
            }
            UpdateTexture(graphicsSystem.virtualScreenTexture, presentBuffer);
        }
    }

    // 3. FINAL PHYSICAL SCREEN BLITTING WITH WINDOW ZOOM SCALING
    BeginDrawing();
        ClearBackground(BLACK);
        
        // We cut out the upright source clipping window straight from our system asset texture slot
        Rectangle src = {
            0.0f, 0.0f,
            (float)graphicsSystem.virtualScreenTexture.width,
            (float)graphicsSystem.virtualScreenTexture.height
        };
         
        // Force target destination box stretching across the upscaled hardware window boundaries
        Rectangle dest = {
            0.0f, 0.0f,
            // (float)config.width  * graphicsSystem.screenScale,
            // (float)config.height * graphicsSystem.screenScale
            (float)graphicsSystem.virtualWidth * graphicsSystem.screenScale, 
            (float)graphicsSystem.virtualHeight * graphicsSystem.screenScale 
        };

        // Render the fully synchronised texture seamlessly onto the monitor viewport
        DrawTexturePro(graphicsSystem.virtualScreenTexture, src, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

void Framework_Cleanup(void) {
    if (kernelState.isAppLoaded && activeApp.Cleanup != NULL) {
        activeApp.Cleanup();
    }

    // Safely unload active streams using structured pointer registers
    if (audioSystem.isMusicActive) {
        UnloadMusicStream(audioSystem.backgroundMusic);
    }
    
    CloseAudioDevice();
    printf("PICO-RAY OS | KERNEL: Structured sound card hardware released cleanly.\n");

    // Safe SpriteRAM reclamation
    if (cartridgeRAM.spriteRAM != NULL) { 
        free(cartridgeRAM.spriteRAM); 
        cartridgeRAM.spriteRAM = NULL; 
    }

    // Safe MapRAM reclamation
    if (cartridgeRAM.mapRAM != NULL) { 
        free(cartridgeRAM.mapRAM); 
        cartridgeRAM.mapRAM = NULL; 
    }

    // Safely erase our active software VRAM and Font Atlas image sectors
    UnloadImage(graphicsSystem.virtualVRAM);
    UnloadImage(graphicsSystem.fontAtlas);
    UnloadImage(graphicsSystem.iconAtlas);
    UnloadTexture(graphicsSystem.virtualScreenTexture);

    if (presentBuffer != NULL) {
        free(presentBuffer);
        presentBuffer = NULL;
        presentBufferSize = 0;
    }
}
