#ifndef APP_SPRITE_EDITOR_H
#define APP_SPRITE_EDITOR_H

#include <stddef.h>

#define SPRITE_EDITOR_MAX_SPRITE_SIZE 64

enum {
    TOOL_PENCIL = 0,
    TOOL_BUCKET_FILL
};

// --- ENCAPSULATED APPLICATION INTERIOR STATE ---
// Packs all module workspace variables into a single unified runtime environment
typedef struct {
    int  spriteSize;
    int  selectedSpriteX;
    int  selectedSpriteY;
    int  sheetViewOffsetX;
    int  sheetViewOffsetY;
    int  sheetDragLastMouseX;
    int  sheetDragLastMouseY;
    bool isSheetDragging;
    int  hoveredPixelX;
    int  hoveredPixelY;
    int  activeColor;
    int  activeToolId;  // Tool tracker (0 = Pencil/Brush, 1 = Bucket Fill)
    bool isGridActive;
    unsigned char copyBuffer[SPRITE_EDITOR_MAX_SPRITE_SIZE][SPRITE_EDITOR_MAX_SPRITE_SIZE]; // Stores pure palette color indices
} SpriteEditorState;

// Unified UI geometry specifications and palette color branding configurations
typedef struct {
    // WORKSPACE CANVAS VIEWPORT BOUNDS
    int canvasX;
    int canvasY;
    int canvasW;
    int canvasH;
    int pixelGridSize;

    // RIGHT-HAND SPRITE SHEET PANEL BOUNDS
    int sheetX;
    int sheetY;
    int sheetW;
    int sheetH;

    // Palette Selector
    int paletteX;
    int paletteY;
    int paletteColorSize;

    // BOTTOM FOOTER STATUS BAR BOUNDS
    int statusX;
    int statusY;
    int statusW;
    int statusH;

    // THEME PALETTE COLOR BRANDING INDEXES (0-31 values)
    int bgCanvasColor;        // Canvas
    int bgCanvasGridColor;    // Canvas Grid
    int bgWorkspaceColor;     // Workspace Background
    int bgStatusBarColor;     // Footer status bar row plate fill
    int textStatusColor;      // Metadata string fonts index color
    int borderSheetColor;     // Right-hand sprite view separating strip
    int paletteSelectorColor;
    int spriteSelectorColor;  // Flashing yellow bounding tracker square box
} SpriteEditorLayout;

void* PR_GetSpriteEditorState(void);

// se-input.c
void SpriteEditorInput_Init(SpriteEditorState *state, const SpriteEditorLayout *layout);
void SpriteEditorInput_SetSpriteSize(int spriteSize);
void SpriteEditorInput_UpdateShortcuts(void);
void SpriteEditorInput_UpdateDrawingBoard(MouseState mousePos);
void SpriteEditorInput_UpdatePaletteSelection(MouseState mousePos);
void SpriteEditorInput_UpdateSpriteSheetSelection(MouseState mousePos);

// se-render.c
void SpriteEditorRender_Init(SpriteEditorState *state, const SpriteEditorLayout *layout);
void SpriteEditorRender_DrawAll(Button *btnCopy, Button *btnPaste, Button *btnToolPencil, Button *btnToolBucket);

// se-shared.c
int SpriteEditorShared_GetStride(const CartridgeRAM *cart);
int SpriteEditorShared_GetRows(const CartridgeRAM *cart);
int SpriteEditorShared_GetSpriteSize(const SpriteEditorState *state);
int SpriteEditorShared_ClampInt(int value, int minValue, int maxValue);
int SpriteEditorShared_GetCanvasCellSize(const SpriteEditorLayout *layout, const SpriteEditorState *state);

// se-storage.c
void SpriteEditorStorage_Init(SpriteEditorState *state);

void SpriteEditorStorage_CommandOpenSheetDialog(void);
void SpriteEditorStorage_CommandSaveSheetDialog(void);
void SpriteEditorStorage_CommandLoadPNG(void);
void SpriteEditorStorage_CommandSavePNG(void);
void SpriteEditorStorage_CommandExportText(void);

// se-tools.c

void SpriteEditorTools_Init(SpriteEditorState *state);

void SpriteEditorTools_CommandCopy(void);
void SpriteEditorTools_CommandPaste(void);
void SpriteEditorTools_ApplyActiveTool(void);


#endif // APP_SPRITE_EDITOR_H
