#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../framework/framework.h"

// Static state holding the isolated instance of the Lua Virtual Machine
static lua_State *L = NULL;


static void LuaRunner_CallHook(const char *hookName);

static bool LuaRunner_LoadAndExecuteCartridge(const char *filePath, bool runInitHook) {
    if (filePath == NULL) return false;

    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL) return false;

    if (!PR_LoadRayCartridge(filePath, cart)) {
        printf("PICO-RAY OS | LUA | ERROR: Failed to load cartridge: %s\n", filePath);
        return false;
    }

    if (cart->luaRAM == NULL || cart->luaRAM[0] == '\0') {
        printf("PICO-RAY OS | LUA | ERROR: Cartridge has no __lua__ section or it is empty.\n");
        return false;
    }

    KernelState *ks = PR_GetKernelState();
    char *finalExecutionBuffer = NULL;

    if (ks != NULL && ks->isPreParserEnabled) {
        finalExecutionBuffer = PR_PreParseLuaSyntax(cart->luaRAM);
        printf("PICO-RAY OS | LUA PIPELINE: Pre-parser enabled.\n");
    } else {
        finalExecutionBuffer = strdup(cart->luaRAM);
        printf("PICO-RAY OS | LUA PIPELINE: Pre-parser bypassed.\n");
    }

    if (finalExecutionBuffer == NULL) {
        printf("PICO-RAY OS | LUA RUNNER ERROR: Failed to prepare executable Lua buffer.\n");
        return false;
    }

    PR_Lua_LoadAPI(L);

    CartridgeMeta *meta = PR_GetCartridgeMeta();
    if (meta != NULL && strcmp(meta->mode, "pico8") == 0) {
        PR_Lua_LoadPico8CompatibilityAPI(L);
        printf("PICO-RAY OS | LUA ENGINE: Loaded PICO-8 compatibility API.\n");
    } else {
        PR_Lua_LoadPicoRayAPI(L);
        printf("PICO-RAY OS | LUA ENGINE: Loaded PICO-RAY API.\n");
    }
    PR_Lua_RegisterAudioAPI(L);

    if (luaL_loadstring(L, finalExecutionBuffer) != LUA_OK || lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        printf("PICO-RAY OS | LUA COMPILE ERROR: %s\n", errorMsg != NULL ? errorMsg : "unknown");
        lua_pop(L, 1);
        free(finalExecutionBuffer);
        return false;
    }

    free(finalExecutionBuffer);

    if (runInitHook) {
        LuaRunner_CallHook("_init");
    }

    return true;
}

static void LuaRunner_CallHook(const char *hookName) {
    if (L == NULL || hookName == NULL) return;

    lua_getglobal(L, hookName);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        printf("PICO-RAY OS | LUA HOOK ERROR (%s): %s\n", hookName, errorMsg != NULL ? errorMsg : "unknown");
        lua_pop(L, 1);
    }
}

static void Command_LuaRun(void) {
    KernelState *ks = PR_GetKernelState();
    
    // A. EXPLICIT UNPAUSE INTERCEPTOR
    // If the game was simply frozen via Pause, instantly release the locks and resume ticks
    // without executing any heavy re-compilations!
    if (L != NULL && ks != NULL && ks->isPauseMenuOpen) {
        ks->isPauseMenuOpen = false;
        ks->isAppLoaded = true;
        // TODO: PR_ResumeAllAudioChannels();
        printf("PICO-RAY | Lua | STATUS: Game resumed from live memory context state cleanly.\n");
        return;
    }

    // We fetch the shared active workspace table address directly!
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->luaRAM == NULL || strlen(cart->luaRAM) == 0) {
        printf("PICO-RAY | Lua | RUN ERROR: RAM luaRAM segment is currently empty!\n");
        return;
    }

    // B. LIVE WORKSPACE HOT-RELOAD PIPELINE
    // Instead of duplicating internal state logic here, we cleanly destroy the old context 
    // and route straight through your existing, production-proven loader block!
    if (L != NULL) {
        lua_close(L);
        L = NULL;
    }
    L = luaL_newstate();
    if (L == NULL) { return; }
    luaL_openlibs(L);

    // Call your unified internal runner engine using the tracked file path!
    // This cleanly registers all core APIs, compatibility tables, and audio blocks in order!
    if (!LuaRunner_LoadAndExecuteCartridge(cart->currentFilePath, true)) {
        lua_close(L);
        L = NULL;
        return;
    }

    if (ks != NULL) {
        ks->isAppLoaded = true;
        ks->isPauseMenuOpen = false;
    }
    printf("PICO-RAY OS | LUA | STATUS: Hot run successful. Game state is running from RAM.\n");
}

static void Command_LuaPause(void) {
    KernelState *ks = PR_GetKernelState();
    if (ks != NULL && ks->isAppLoaded) {
        ks->isAppLoaded = false;    // Freezes update ticks loops instantly
        ks->isPauseMenuOpen = true;  // Flag system context freeze state active
        // TODO: PR_PauseAllAudioChannels(); 
        printf("PICO-RAY OS| LUA | STATUS: Game loop frozen via Pause. Lua VM remains active in RAM.\n");
    }
}

static void Command_LuaStop(void) {
    KernelState *ks = PR_GetKernelState();
    if (ks != NULL) {
        ks->isAppLoaded = false;
        ks->isPauseMenuOpen = false;
    }

    if (L != NULL) {
        lua_close(L);
        L = NULL;
        printf("PICO-RAY | LUA | STATUS: Lua Virtual Machine closed down cleanly.\n");
    }
    // TODO: PR_MuteAllAudioChannels(); 
    // TODO: PR_RebuildMenuBar();
}

// DYNAMIC RELOADER CALLBACK: Triggered instantly when user selects a file inside the central UI Open Dialog!
void Callback_OnCartridgeSelected(const char *filePath) {
    if (filePath == NULL) return;

    printf("PICO-RAY OS | LIVE RELOADER: Switching execution runtime channel to %s\n", filePath);

    // 1. RE-BOOTSTRAP CENTRAL LUA VIRTUAL MACHINE INSTANCE
    if (L != NULL) {
        lua_close(L);
        L = NULL;
    }

    L = luaL_newstate();
    if (L == NULL) {
        printf("PICO-RAY OS | LUA RUNNER CRITICAL ERROR: Failed to allocate new Lua State state machine instance!\n");
        return;
    }

    // Open standard core execution library tables (math, table, string, etc.)
    luaL_openlibs(L);

    if (!LuaRunner_LoadAndExecuteCartridge(filePath, true)) {
        lua_close(L);
        L = NULL;
        return;
    }

    // App state loaded confirmation flag on
    KernelState *ks = PR_GetKernelState();
    if (ks != NULL) {
        ks->isAppLoaded = true;
    }

    printf("PICO-RAY OS | LUA ENGINE: Cartridge initialized and marked as active app context.\n");
}

// Hard forced reset: cleans memory states and forces a complete cold boot re-read from disk
static void Command_LuaReload(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();

    if (cart == NULL || strlen(cart->currentFilePath) == 0) {
        printf("PICO-RAY | LUA | RELOAD ERROR: No active cartridge filename register mapped inside RAM!\n");
        return;
    }
    
    printf("PICO-RAY | LUA | FORCE COLD RELOADING FILE FROM DISK SOURCE PIPELINE...\n");
    
    KernelState *ks = PR_GetKernelState();
    if (ks != NULL) {
        ks->isAppLoaded = false;
        ks->isPauseMenuOpen = false; // Turn off to force parser to overwrite graphics!
    }
    
    // Re-trigger using the centrally stored file path token string
    Callback_OnCartridgeSelected(cart->currentFilePath);
}

static void Command_ShowCartMetaInfo(void) {
    // Triggers the pop up window dialog we just polished!
    PR_DrawAboutWindow(); 
}

// Action command opening the unified core framework file open panel layout
static void Command_OpenCartridgeFileDialog(void) {
    // Open Dialog scanning strictly inside the 'carts' folder targeting '.ray' file extensions!
    // Passes our dynamic reloader callback address hook as the event payload!
    PR_OpenFileDialog("carts", ".ray", Callback_OnCartridgeSelected);
}

// --- CORE LIFE-CYCLE APPLICATION INTERFACE IMPLEMENTATION ---

static AppConfig LuaRunner_GetConfig(void) {
    AppConfig config = {
        .name       = "Lua Runner",
        .author     = "Gergely Macoun,\nAI_Collaborator",
        .version    = "0.1.0",
        .license    = "MIT",
        .appId      = (unsigned int)SYS_APP_LUA_RUNNER,
        .iconId     = ICON_LUA,
        .width      = 128,
        .height     = 128,
        .hasMenuBar = true
    };
    return config;
}

static void LuaRunner_RegisterMenus(void) {
    Menu *fileMenu = PR_GetRegisteredMenu("File");

    if (fileMenu != NULL) {
        PR_AddMenuItem(fileMenu, "Load Cart", ICON_FOLDER, Command_OpenCartridgeFileDialog);        
    }

    Menu cartMenu = PR_CreateMenu("Cart");
    PR_AddMenuItem(&cartMenu, "Run        [F5]",  ICON_PLAY,   Command_LuaRun);
    PR_AddMenuItem(&cartMenu, "Pause      [F5]",  ICON_PAUSE,  Command_LuaPause);
    PR_AddMenuItem(&cartMenu, "Stop/Reset [F6]",  ICON_STOP,   Command_LuaStop);
    PR_AddMenuItem(&cartMenu, "Reload File [F7]", ICON_RELOAD, Command_LuaReload);
    // PR_AddMenuSeparator(&cartMenu);
    PR_AddMenuItem(&cartMenu, "Cartridge Info",   ICON_INFO,   Command_ShowCartMetaInfo);

    PR_RegisterApplicationMenu(cartMenu);
}

static void LuaRunner_Init(void) {
    // Fire up a fresh, isolated Lua processing state machine
    L = luaL_newstate();
    if (L == NULL) {
        printf("LUA INTERFACE ERROR: Failed to instantiate core interpreter engine state.\n");
        return;
    }

    // Mount basic embedded mathematical and utility script libraries
    luaL_openlibs(L);

    const char *cartPath = getenv("PICORAY_CART");
    if (cartPath == NULL || cartPath[0] == '\0') {
        cartPath = "carts/game.ray";
    }

    if (!LuaRunner_LoadAndExecuteCartridge(cartPath, true)) {
        lua_close(L);
        L = NULL;
        return;
    }
}

static void LuaRunner_Update(MouseState mousePos) {
    (void)mousePos;
    // Execute active script logical loop 60 frames per second inside the framework tick thread
    LuaRunner_CallHook("_update");
}

static void LuaRunner_Draw(void) {
    // Direct rendering layer: redirects Lua graphic instructions straight to active screen buffer
    LuaRunner_CallHook("_draw");
}

static void LuaRunner_Cleanup(void) {
    // Safe hardware reclamation cycle: dissolve Lua instance memory cleanly upon application exit
    if (L != NULL) {
        lua_close(L);
        L = NULL;
    }
}

// --- THE OFFICIAL PUBLIC INTERFACE PACKAGE EXPORT ---
AppInterface LuaRunner_App = {
    .GetConfig     = LuaRunner_GetConfig,
    .Init          = LuaRunner_Init,
    .RegisterMenus = LuaRunner_RegisterMenus,
    .Update        = LuaRunner_Update,
    .Draw          = LuaRunner_Draw,
    .Cleanup       = LuaRunner_Cleanup
};
