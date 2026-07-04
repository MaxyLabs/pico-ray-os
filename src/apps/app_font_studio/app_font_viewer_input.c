#include <string.h>
#include "app_font_viewer.h"

static int Input_GetTotalRowsNeeded(const FontViewerState *state) {
    return state->isFullMapMode ? 16 : 6;
}

static int Input_GetGlyphWidth(const FontRegistry *font, int asciiCode) {
    int width = (asciiCode >= 32 && asciiCode <= 126) ? font->printableWidth : font->charWidth;
    if (width < 0) return 0;
    if (width > 8) return 8;
    return width;
}

static int Input_GetGlyphHeight(const FontRegistry *font) {
    if (font->charHeight < 0) return 0;
    if (font->charHeight > FONT_VIEWER_MAX_GLYPH_ROWS) return FONT_VIEWER_MAX_GLYPH_ROWS;
    return font->charHeight;
}

static int Input_GetGridCellHeight(const FontRegistry *font) {
    int glyphHeight = Input_GetGlyphHeight(font);
    int cellH = glyphHeight + 1;
    if (cellH < 6) cellH = 6;
    return cellH;
}

static int Input_GetVisibleGridRows(const FontViewerLayout *layout, const FontRegistry *font) {
    int topPadding = layout->gridStartY - layout->gridBoxY;
    if (topPadding < 0) topPadding = 0;

    int availableH = layout->gridBoxH - topPadding;
    int cellH = Input_GetGridCellHeight(font);
    if (cellH <= 0) return 1;

    int rows = availableH / cellH;
    if (rows < 1) rows = 1;
    if (rows > 16) rows = 16;
    return rows;
}

static int Input_GetDynamicGridBoxHeight(const FontViewerState *state, const FontViewerLayout *layout, const FontRegistry *font) {
    int topPadding = layout->gridStartY - layout->gridBoxY;
    if (topPadding < 0) topPadding = 0;

    int cellH = Input_GetGridCellHeight(font);
    int rowsNeeded = Input_GetTotalRowsNeeded(state);

    int desired = topPadding + (rowsNeeded * cellH) + 2;
    int availableTotalH = 120 - layout->gridBoxY;
    int lowerMinH = 42;
    int maxH = availableTotalH - lowerMinH;
    if (maxH < 32) maxH = 32;

    int minH = topPadding + cellH + 2;
    if (desired < minH) desired = minH;
    if (desired > maxH) desired = maxH;

    return desired;
}

static int Input_GetVisibleGridRowsForBox(const FontViewerLayout *layout, const FontRegistry *font, int gridBoxHeight) {
    int topPadding = layout->gridStartY - layout->gridBoxY;
    if (topPadding < 0) topPadding = 0;

    int availableH = gridBoxHeight - topPadding;
    int cellH = Input_GetGridCellHeight(font);
    if (cellH <= 0) return 1;

    int rows = availableH / cellH;
    if (rows < 1) rows = 1;
    if (rows > 16) rows = 16;
    return rows;
}

static void Input_UpdateScroll(FontViewerState *state, const FontViewerLayout *layout) {
    float wheelMove = GetMouseWheelMove();
    if (wheelMove < 0.0f || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        state->scrollRowOffset++;
    }
    if (wheelMove > 0.0f || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        if (state->scrollRowOffset > 0) state->scrollRowOffset--;
    }

    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;
    FontRegistry font = fontData[activeFontId];
    int gridBoxH = Input_GetDynamicGridBoxHeight(state, layout, &font);
    int visibleRows = Input_GetVisibleGridRowsForBox(layout, &font, gridBoxH);

    int totalRowsNeeded = Input_GetTotalRowsNeeded(state);
    int maxScrollOffset = totalRowsNeeded - visibleRows;
    if (maxScrollOffset < 0) maxScrollOffset = 0;
    if (state->scrollRowOffset > maxScrollOffset) state->scrollRowOffset = maxScrollOffset;
}

static bool Input_ApplyPreviewPaint(FontViewerState *state, const FontViewerLayout *layout, int previewGridY, MouseState mousePos) {
    if (state->selectedAscii == -1 || !state->isEditMode) return false;

    bool leftDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool rightDown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    if (!leftDown && !rightDown) return false;

    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;

    FontRegistry font = fontData[activeFontId];
    int activeWidth = Input_GetGlyphWidth(&font, state->selectedAscii);
    int activeHeight = Input_GetGlyphHeight(&font);

    int clickX = (int)((layout->previewRightAnchorX + layout->previewBlockSize - mousePos.x) / layout->previewBlockGap);
    int clickY = (int)((mousePos.y - previewGridY) / layout->previewBlockGap);

    if (clickX < 0 || clickX >= activeWidth || clickY < 0 || clickY >= activeHeight) {
        return false;
    }

    unsigned char clickMask = (unsigned char)(1u << clickX);
    unsigned char beforeRow = state->editBuffer[clickY];

    if (rightDown) {
        state->editBuffer[clickY] &= (unsigned char)~clickMask;
    } else {
        // Paint mode: while holding left mouse, continuously set active pixels.
        state->editBuffer[clickY] |= clickMask;
    }

    if (beforeRow != state->editBuffer[clickY]) {
        memcpy(state->customGlyphs[activeFontId][state->selectedAscii], state->editBuffer, sizeof(state->editBuffer));
        state->hasChanged[activeFontId][state->selectedAscii] = true;
    }

    return true;
}

static void Input_UpdateHoveredCell(FontViewerState *state, const FontViewerLayout *layout, MouseState mousePos) {
    state->hoveredAscii = -1;

    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;
    FontRegistry font = fontData[activeFontId];
    int gridBoxH = Input_GetDynamicGridBoxHeight(state, layout, &font);
    int visibleRows = Input_GetVisibleGridRowsForBox(layout, &font, gridBoxH);
    int cellH = Input_GetGridCellHeight(&font);
    int cellW = layout->gridBoxW / 16;
    if (cellW < 1) cellW = 1;

    int totalRowsNeeded = Input_GetTotalRowsNeeded(state);
    int baseOffset = state->isFullMapMode ? 0 : 32;

    for (int y = 0; y < visibleRows; y++) {
        int actualRowIndex = state->scrollRowOffset + y;
        if (actualRowIndex >= totalRowsNeeded) break;

        for (int x = 0; x < 16; x++) {
            int targetAscii = baseOffset + (actualRowIndex * 16) + x;
            if (state->isFullMapMode && targetAscii > 255) break;
            if (!state->isFullMapMode && targetAscii > 126) break;

            float cx = (float)layout->gridStartX + (x * cellW);
            float cy = (float)layout->gridStartY + (y * cellH);

            if (mousePos.x >= cx && mousePos.x < cx + cellW &&
                mousePos.y >= cy && mousePos.y < cy + cellH) {
                state->hoveredAscii = targetAscii;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !state->isEditMode) {
                    FontViewerCommands_LockSelection(state, targetAscii);
                }
                return;
            }
        }
    }
}

void FontViewerInput_Update(FontViewerState *state, FontViewerButtons *buttons, const FontViewerLayout *layout, MouseState mousePos) {
    if (state == NULL || buttons == NULL || layout == NULL) return;

    MenuSystem *menu = PR_GetMenuSystem();
    if (menu != NULL && menu->activeMenuIndex > 0) {
        state->hoveredAscii = -1;
        return;
    }

    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;
    FontRegistry font = fontData[activeFontId];
    int gridBoxH = Input_GetDynamicGridBoxHeight(state, layout, &font);
    int lowerPanelY = layout->gridBoxY + gridBoxH + 1;
    if (lowerPanelY < 66) lowerPanelY = 66;
    int previewGridY = lowerPanelY + 10;

    //buttons->btnToggleMode.x = 106;
    buttons->btnToggleMode.y = 120;
    //buttons->btnEditMode.x = 4;
    buttons->btnEditMode.y = lowerPanelY + 4;
    //buttons->btnPrintGlyph.x = 22;
    buttons->btnPrintGlyph.y = lowerPanelY + 4;
    PR_UpdateButton(&buttons->btnToggleMode, mousePos);
    PR_UpdateButton(&buttons->btnEditMode, mousePos);
    PR_UpdateButton(&buttons->btnPrintGlyph, mousePos);

    Input_UpdateScroll(state, layout);

    if (IsKeyPressed(KEY_ESCAPE)) {
        FontViewerCommands_UnlockSelection(state);
    }

    // Quick return to hover mode: right-click clears locked selection when not editing.
    if (!state->isEditMode && state->selectedAscii != -1 && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        FontViewerCommands_UnlockSelection(state);
    }

    if (Input_ApplyPreviewPaint(state, layout, previewGridY, mousePos)) {
        return;
    }

    Input_UpdateHoveredCell(state, layout, mousePos);
}
