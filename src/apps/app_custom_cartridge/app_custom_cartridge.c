#include "../../framework/framework.h"

// --- HIVATALOS EXPORT JELZÉSEK macOS / LINUX ALATT ---
#define EXPORT __attribute__((visibility("default")))

EXPORT const int PR_PLUGIN_ABI = PR_PLUGIN_ABI_VERSION;

// Fontos: a függvények elé érdemes kitenni az export jelzést, hogy kívülről láthatóak legyenek
EXPORT AppConfig Plugin_GetConfig(void) {
    AppConfig config = {
        .name       = "Custom Plugin Cartridge",
        .author     = "Gergely Macoun,\nAI_Collaborator",
        .version    = "0.1.0",
        .license    = "MIT",
        // .appId = #,
        .iconId     = ICON_PLUGIN,
        .width      = 128,
        .height     = 128,
        .hasMenuBar = true
    };
    return config;
}

void Plugin_RegisterMenus(void) {

}

EXPORT void Plugin_Init(void) {
    PR_Cls(P8_BLACK);
}

EXPORT void Plugin_Update(MouseState mousePos) {
    // Játékmenet logika...
    if (IsKeyPressed(KEY_ESCAPE)) {
        PR_Callback_CloseApp();
        return;
    }
}

EXPORT void Plugin_Draw(void) {
    PR_Cls(P8_BLACK);
    PR_RectFill(20, 20, 30, 30, P8_RED);
    PR_Print("Third Party Game!", 25, 25, P8_WHITE);
}

EXPORT void Plugin_Cleanup(void) {

}

// --- THE OFFICIAL PUBLIC INTERFACE PACKAGE ---
// This bundles the entire module lifecycle into a single exportable structural object
EXPORT const AppInterface Plugin_App = {
    .GetConfig     = Plugin_GetConfig,
    .Init          = Plugin_Init,
    .RegisterMenus = Plugin_RegisterMenus,
    .Update        = Plugin_Update,
    .Draw          = Plugin_Draw,
    .Cleanup       = Plugin_Cleanup
};
