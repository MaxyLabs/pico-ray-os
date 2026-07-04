#ifndef APP_FONT_VIEWER_H
#define APP_FONT_VIEWER_H

#include "../../framework/framework.h"

typedef struct {
    int  selectedAscii;
    int  hoveredAscii;
    bool isFullMapMode;
    int  scrollRowOffset;
    bool isEditMode;
    unsigned char editBuffer[FONT_VIEWER_MAX_GLYPH_ROWS];

    // Keep edits isolated per font so switching fonts does not mix buffers.
    bool hasChanged[FONT_COUNT][256];
    unsigned char customGlyphs[FONT_COUNT][256][FONT_VIEWER_MAX_GLYPH_ROWS];
} FontViewerState;

typedef struct {
    Button btnToggleMode;
    Button btnEditMode;
    Button btnPrintGlyph;
} FontViewerButtons;

typedef struct {
    int gridBoxX;
    int gridBoxY;
    int gridBoxW;
    int gridBoxH;
    int gridStartX;
    int gridStartY;
    int gridCellW;
    int gridCellH;
    int previewRightAnchorX;
    int previewGridY;
    int previewBlockSize;
    int previewBlockGap;
    int visibleGridRows;
} FontViewerLayout;

AppConfig FontViewer_GetConfig(void);
void FontViewer_RegisterMenus(void);
void FontViewer_Init(void);
void FontViewer_Update(MouseState mousePos);
void FontViewer_Draw(void);
void FontViewer_Cleanup(void);

void FontViewerCommands_LockSelection(FontViewerState *state, int asciiCode);
void FontViewerCommands_UnlockSelection(FontViewerState *state);
void FontViewerCommands_ToggleEditMode(FontViewerState *state);
void FontViewerCommands_ToggleViewMode(FontViewerState *state);
void FontViewerCommands_PrintGlyphToTerminal(const FontViewerState *state);
void FontViewerCommands_ExportAllModifiedGlyphs(const FontViewerState *state);

void FontViewerInput_Update(FontViewerState *state, FontViewerButtons *buttons, const FontViewerLayout *layout, MouseState mousePos);

void FontViewerRender_Draw(const FontViewerState *state, const FontViewerLayout *layout);

#endif // APP_FONT_VIEWER_H
