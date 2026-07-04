#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../framework/framework.h"
#include "app_sprite_editor.h"
// #include "se_storage.h"
// #include "se_tools.h"
// #include "se_render.h"
// #include "se_input.h"

static SpriteEditorState state; // Sealed dynamic application execution context

static const SpriteEditorLayout layout = {
    // Canvas bounds
    .canvasX          = 3,
    .canvasY          = 12,
    .canvasW          = 97,
    .canvasH          = 97,
    .pixelGridSize    = 12,

    // Sprite Sheet panel bounds
    .sheetX           = 128,
    .sheetY           = 8,
    .sheetW           = 128,
    .sheetH           = 128,

    // Palette Selector
    .paletteX         = 4,
    .paletteY         = 112,
    .paletteColorSize = 5,

    // Bottom Status Bar bounds
    .statusX          = 0,
    .statusY          = 128,
    .statusW          = 128,
    .statusH          = 8,

    // Theme Color Brandings (Palette index tokens mapping)
    .bgCanvasColor        = P8_BLACK,
    .bgCanvasGridColor    = P8_DARK_GREY,
    .bgWorkspaceColor     = P8_DARK_BLUE,
    .bgStatusBarColor     = P8_RED,
    .textStatusColor      = P8_DARK_PURPLE,
    .borderSheetColor     = P8_BLACK,
    .paletteSelectorColor = P8_WHITE,
    .spriteSelectorColor  = P8_YELLOW
};

// Persistent graphical button nodes managed inside the local stack
static Button btnCopy;
static Button btnPaste;
static Button btnToolPencil;
static Button btnToolBucket;

static void Callback_SelectPencil(void) { state.activeToolId = TOOL_PENCIL; printf("PICO-RAY OS | SPRITE EDITOR | Tool: PENCIL\n"); }
static void Callback_SelectBucket(void) { state.activeToolId = TOOL_BUCKET_FILL; printf("PICO-RAY OS | SPRITE EDITOR | Tool: BUCKET FILL\n"); }
static void Callback_SetSpriteSize8(void)  { SpriteEditorInput_SetSpriteSize(8); }
static void Callback_SetSpriteSize16(void) { SpriteEditorInput_SetSpriteSize(16); }
static void Callback_SetSpriteSize32(void) { SpriteEditorInput_SetSpriteSize(32); }

void* PR_GetSpriteEditorState(void) {
    return (void*)&state; // Return raw address of our sealed editor state capsule
}

static void Callback_ToggleGrid(void) {
    state.isGridActive = !state.isGridActive;
}

// --- HARDWARE STATE UPDATE ENGINE PIPELINES ---

// --- SYSTEM CENTRAL LIFE-CYCLE INTERFACE IMPLEMENTATIONS ---
AppConfig SpriteEditor_GetConfig(void) {
    AppConfig config = {
        .name       = "Sprite Editor",
        .author     = "Gergely Macoun,\nAI_Collaborator",
        .version    = "0.1.0",
        .license    = "MIT",
        .appId      = (unsigned int)SYS_APP_SPRITE_EDITOR,
        .iconId     = ICON_SPRITE_EDITOR,
        .width      = 256,
        .height     = 128 + 8,
        .hasMenuBar = true
    };
    return config;
}

void SpriteEditor_RegisterMenus(void) {
    Menu *fileMenu = PR_GetRegisteredMenu("File");
    if (fileMenu != NULL) {
        PR_AddMenuItem(fileMenu, "Load",       ICON_FOLDER, SpriteEditorStorage_CommandLoadPNG);
        PR_AddMenuItem(fileMenu, "Save",       ICON_EMPTY,  SpriteEditorStorage_CommandSavePNG);
        PR_AddMenuItem(fileMenu, "Open Sheet", ICON_FOLDER, SpriteEditorStorage_CommandOpenSheetDialog);
        PR_AddMenuItem(fileMenu, "Save Sheet", ICON_SAVE,   SpriteEditorStorage_CommandSaveSheetDialog);
        PR_AddMenuItem(fileMenu, "Export TXT", ICON_EMPTY,  SpriteEditorStorage_CommandExportText);
    }
    
    Menu editMenu = PR_CreateMenu("Edit");
    PR_AddMenuItem(&editMenu, "Copy",  -1, SpriteEditorTools_CommandCopy);
    PR_AddMenuItem(&editMenu, "Paste", -1, SpriteEditorTools_CommandPaste);
    PR_RegisterApplicationMenu(editMenu);

    Menu toolsMenu = PR_CreateMenu("Tools");
    PR_AddMenuItem(&toolsMenu, "Sprite x8",   -1, Callback_SetSpriteSize8);
    PR_AddMenuItem(&toolsMenu, "Sprite x16",  -1, Callback_SetSpriteSize16);
    PR_AddMenuItem(&toolsMenu, "Sprite x32",  -1, Callback_SetSpriteSize32);
    PR_AddMenuItem(&toolsMenu, "Toggle Grid", -1, Callback_ToggleGrid);
    PR_RegisterApplicationMenu(toolsMenu);
}

void SpriteEditor_Init(void) {
    memset(&state, 0, sizeof(SpriteEditorState));
    SpriteEditorStorage_Init(&state);
    SpriteEditorTools_Init(&state);
    SpriteEditorRender_Init(&state, &layout);
    SpriteEditorInput_Init(&state, &layout);

    state.spriteSize = 8;
    state.activeColor = 8;
    state.sheetViewOffsetX = 0;
    state.sheetViewOffsetY = 0;
    state.sheetDragLastMouseX = 0;
    state.sheetDragLastMouseY = 0;
    state.isSheetDragging = false;
    state.hoveredPixelX = -1;
    state.hoveredPixelY = -1;
    SpriteEditorInput_SetSpriteSize(8);

    btnCopy  = PR_CreateButton(104, 14, 20, 8, "CPY", -1,          SpriteEditorTools_CommandCopy);
    btnPaste = PR_CreateButton(104, 24, 20, 8, "PST", -1,          SpriteEditorTools_CommandPaste);
    // btnLoad  = PR_CreateButton(104, 36, 20, 8, "",    ICON_FOLDER, Command_Load);
    // btnSave  = PR_CreateButton(104, 46, 20, 8, "SAV", -1,          Command_Save);
    btnToolPencil = PR_CreateButton(104, 36, 10, 10, "", ICON_PENCIL, Callback_SelectPencil);
    btnToolBucket = PR_CreateButton(116, 36, 10, 10, "", ICON_FLOOD,  Callback_SelectBucket);
}

void SpriteEditor_Update(MouseState mousePos) {
    SpriteEditorInput_UpdateShortcuts();
    
    MenuSystem *menu = PR_GetMenuSystem();
    if (PR_IsFileDialogOpen() || (menu != NULL && menu->activeMenuIndex > 0)) {
        state.hoveredPixelX = -1;
        state.hoveredPixelY = -1;
        return;
    }
    
    PR_UpdateButton(&btnCopy,  mousePos);
    PR_UpdateButton(&btnPaste, mousePos);
    // PR_UpdateButton(&btnLoad,  mousePos);
    // PR_UpdateButton(&btnSave,  mousePos);
    PR_UpdateButton(&btnToolPencil, mousePos);
    PR_UpdateButton(&btnToolBucket, mousePos);

    SpriteEditorInput_UpdateDrawingBoard(mousePos);
    SpriteEditorInput_UpdatePaletteSelection(mousePos);
    SpriteEditorInput_UpdateSpriteSheetSelection(mousePos);
}

void SpriteEditor_Draw(void) {
    PR_Cls(P8_BLACK);
    SpriteEditorRender_DrawAll(&btnCopy, &btnPaste, &btnToolPencil, &btnToolBucket);
}

void SpriteEditor_Cleanup(void) {
}

// --- THE OFFICIAL PUBLIC INTERFACE PACKAGE ---
AppInterface SpriteEditor_App = {
    .GetConfig     = SpriteEditor_GetConfig,
    .Init          = SpriteEditor_Init,
    .RegisterMenus = SpriteEditor_RegisterMenus,
    .Update        = SpriteEditor_Update,
    .Draw          = SpriteEditor_Draw,
    .Cleanup       = SpriteEditor_Cleanup
};
