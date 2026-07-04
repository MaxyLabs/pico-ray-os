#include <string.h>
#include "app_font_viewer.h"

static FontViewerState state;
static FontViewerButtons buttons;

static const FontViewerLayout layout = {
    .gridBoxX = 0,
    .gridBoxY = 11,
    .gridBoxW = 128,
    .gridBoxH = 64,
    .gridStartX = 0,
    .gridStartY = 14,
    .gridCellW = 8,
    .gridCellH = 7,
    .previewRightAnchorX = 32,
    .previewGridY = 80,
    .previewBlockSize = 3,
    .previewBlockGap = 4,
    .visibleGridRows = 7
};

static void Callback_ToggleMode(void) {
    FontViewerCommands_ToggleViewMode(&state);
}

static void Callback_ToggleEdit(void) {
    FontViewerCommands_ToggleEditMode(&state);
}

static void Callback_PrintGlyph(void) {
    FontViewerCommands_PrintGlyphToTerminal(&state);
}

static void Callback_ExportAll(void) {
    FontViewerCommands_ExportAllModifiedGlyphs(&state);
}

AppConfig FontViewer_GetConfig(void) {
    AppConfig config = {
        .name       = "Font Studio",
        .author     = "Gergely Macoun,\nAI_Collaborator",
        .version    = "0.1.0",
        .license    = "MIT",
        .appId      = (unsigned int)SYS_APP_FONT_STUDIO,
        .iconId     = ICON_FONT_STUDIO,
        .width      = 128,
        .height     = 128,
        .hasMenuBar = true
    };
    return config;
}

void FontViewer_RegisterMenus(void) {
    Menu *fileMenu = PR_GetRegisteredMenu("File");
    if (fileMenu != NULL) {
        PR_AddMenuItem(fileMenu, "Export All", -1, Callback_ExportAll);
    }

    Menu toolsMenu = PR_CreateMenu("Tools");
    PR_AddMenuItem(&toolsMenu, "Toggle Mode", -1, Callback_ToggleMode);
    PR_RegisterApplicationMenu(toolsMenu);
}

void FontViewer_Init(void) {
    memset(&state, 0, sizeof(state));
    state.hoveredAscii = -1;
    state.selectedAscii = -1;
    state.isFullMapMode = false;
    state.scrollRowOffset = 0;

    // buttons.btnToggleMode = PR_CreateButton(106, 120, 18, 8, "MD", -1, Callback_ToggleMode);
    // buttons.btnEditMode = PR_CreateButton(4, 108, 16, 8, "EDT", -1, Callback_ToggleEdit);
    //buttons.btnPrintGlyph = PR_CreateButton(48, 108, 16, 8, "PRT", -1, Callback_PrintGlyph);

    buttons.btnToggleMode = PR_CreateButton(106, 112, 18, 8, "MD", -1, Callback_ToggleMode);
    buttons.btnEditMode   = PR_CreateButton(60, 80, 16, 8, "EDT", -1, Callback_ToggleEdit);
    buttons.btnPrintGlyph = PR_CreateButton(72, 80, 16, 8, "PRT", -1, Callback_PrintGlyph);
}

void FontViewer_Update(MouseState mousePos) {
    FontViewerInput_Update(&state, &buttons, &layout, mousePos);
}

void FontViewer_Draw(void) {
    FontViewerRender_Draw(&state, &layout);

    PR_DrawButton(&buttons.btnToggleMode);
    PR_DrawButton(&buttons.btnEditMode);
    PR_DrawButton(&buttons.btnPrintGlyph);
}

void FontViewer_Cleanup(void) {
}

AppInterface FontViewer_App = {
    .GetConfig = FontViewer_GetConfig,
    .Init = FontViewer_Init,
    .RegisterMenus = FontViewer_RegisterMenus,
    .Update = FontViewer_Update,
    .Draw = FontViewer_Draw,
    .Cleanup = FontViewer_Cleanup
};
