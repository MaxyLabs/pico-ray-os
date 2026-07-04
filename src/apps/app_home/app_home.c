#include "raylib.h"
#include <stdio.h>
#include "../../framework/framework.h"

// External reference to tell the Home screen that oher app modules exists
extern AppInterface FontViewer_App;
extern AppInterface SpriteEditor_App;
extern AppInterface Terminal_App;

extern AppInterface LuaRunner_App;
extern AppInterface Plugin_App;

// --- Callback ---
// Clean static callback wrappers that fire application context switches instantly
static void Callback_LaunchFontViewer(void) {
    SwitchApp(FontViewer_App);
}

static void Callback_LaunchSpriteEditor(void) {
    SwitchApp(SpriteEditor_App);
}

static void Callback_LaunchTerminal(void) {
    SwitchApp(Terminal_App);
}

static void Callback_LaunchLuaRunner(void) {
    SwitchApp(LuaRunner_App);
}

static void Callback_LoadPlugin(void) {
    PR_LoadPluginCartridge();
}

AppConfig Home_GetConfig(void) {
    AppConfig config = {
        .name       = "Home",
        .author     = "Gergely Macoun,\nAI_Collaborator",
        .version    = "0.1.0",
        .license    = "MIT",
        .appId      = (unsigned int)SYS_APP_HOME,
        .iconId     = ICON_HOME,
        .width      = 256,
        .height     = 128,
        .hasMenuBar = true,
    };
    return config;
}

void Home_RegisterMenus(void) {
    // 1. CREATE AN "APPS" NAVIGATION TAB FOR QUICK LAUNCHING
    Menu appsMenu = PR_CreateMenu("Apps");
    Menu cartMenu = PR_CreateMenu("Cart");
    
    // FIX: Pass the exact C function pointer names directly as event callback targets!
    // No more dirty action ID integers bubbling up through variables
    PR_AddMenuItem(&appsMenu, "Font Studio",   ICON_FONT_STUDIO,   Callback_LaunchFontViewer);
    PR_AddMenuItem(&appsMenu, "Sprite Editor", ICON_SPRITE_EDITOR, Callback_LaunchSpriteEditor);
    PR_AddMenuItem(&appsMenu, "Terminal",      ICON_TERMINAL,      Callback_LaunchTerminal);

    PR_AddMenuItem(&cartMenu, "Lua Runner",    ICON_LUA,    Callback_LaunchLuaRunner);
    PR_AddMenuItem(&cartMenu, "Plugin Runner", ICON_PLUGIN, Callback_LoadPlugin);
    
    // 2. REGISTER THE SYSTEM TOP BAR COMPONENT
    PR_RegisterApplicationMenu(appsMenu);
    PR_RegisterApplicationMenu(cartMenu);
}

void Home_Init(void) {
}

void Home_Update(MouseState mousePos) {
}

void Home_Draw(void) {
    PR_Cls(P8_DARK_BLUE);

    // RENDER BACKGROUND ENVIRONMENT (Desktop Workspace Center)
    PR_Print("PICO-RAY OPERATING SYSTEM", 55, 50, P8_WHITE);
    PR_Print("Click on 'Apps' menu to launch a module.", 32, 65, P8_LIGHT_GREY);
}

void Home_Cleanup(void) {
    // No dynamic memory allocations requiring standard memory tracking or cleanup cycles here
}

// --- THE OFFICIAL PUBLIC HOME INTERFACE PACKAGE ---
// Bundles the entire home screen dashboard lifecycle into a single structural object
AppInterface Home_App = {
    .GetConfig     = Home_GetConfig,
    .Init          = Home_Init,
    .RegisterMenus = Home_RegisterMenus,
    .Update        = Home_Update,
    .Draw          = Home_Draw,
    .Cleanup       = Home_Cleanup
};
