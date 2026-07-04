#include "app_font_viewer.h"

static int Render_GetTotalRowsNeeded(const FontViewerState *state) {
    return state->isFullMapMode ? 16 : 6;
}

static int Render_GetActiveAscii(const FontViewerState *state) {
    if (state->selectedAscii != -1) return state->selectedAscii;
    return state->hoveredAscii;
}

static int Render_GetGlyphWidth(const FontRegistry *font, int asciiCode) {
    int width = (asciiCode >= 32 && asciiCode <= 126) ? font->printableWidth : font->charWidth;
    if (width < 0) return 0;
    if (width > 8) return 8;
    return width;
}

static int Render_GetGlyphHeight(const FontRegistry *font) {
    if (font->charHeight < 0) return 0;
    if (font->charHeight > FONT_VIEWER_MAX_GLYPH_ROWS) return FONT_VIEWER_MAX_GLYPH_ROWS;
    return font->charHeight;
}

static int Render_GetGridCellHeight(const FontRegistry *font) {
    int glyphHeight = Render_GetGlyphHeight(font);
    int cellH = glyphHeight + 1;
    if (cellH < 6) cellH = 6;
    return cellH;
}

static int Render_GetVisibleGridRows(const FontViewerLayout *layout, const FontRegistry *font) {
    int topPadding = layout->gridStartY - layout->gridBoxY;
    if (topPadding < 0) topPadding = 0;

    int availableH = layout->gridBoxH - topPadding;
    int cellH = Render_GetGridCellHeight(font);
    if (cellH <= 0) return 1;

    int rows = availableH / cellH;
    if (rows < 1) rows = 1;
    if (rows > 16) rows = 16;
    return rows;
}

static int Render_GetDynamicGridBoxHeight(const FontViewerState *state, const FontViewerLayout *layout, const FontRegistry *font) {
    int topPadding = layout->gridStartY - layout->gridBoxY;
    if (topPadding < 0) topPadding = 0;

    int cellH = Render_GetGridCellHeight(font);
    int rowsNeeded = Render_GetTotalRowsNeeded(state);

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

static int Render_GetVisibleGridRowsForBox(const FontViewerLayout *layout, const FontRegistry *font, int gridBoxHeight) {
    int topPadding = layout->gridStartY - layout->gridBoxY;
    if (topPadding < 0) topPadding = 0;

    int availableH = gridBoxHeight - topPadding;
    int cellH = Render_GetGridCellHeight(font);
    if (cellH <= 0) return 1;

    int rows = availableH / cellH;
    if (rows < 1) rows = 1;
    if (rows > 16) rows = 16;
    return rows;
}

static const unsigned char *Render_FindGlyphBytes(unsigned int codepoint) {
    int activeFontId = PR_GetActiveFontId();
    
    // Safety proxy link straight to our newly unified hardware dictionary engine pass
    return PR_GetFontGlyph(activeFontId, codepoint);
}

static void Render_MagnifiedFont(const FontViewerState *state, const FontViewerLayout *layout, int previewGridY) {
    int activeAscii = Render_GetActiveAscii(state);
    if (activeAscii == -1) return;

    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;

    FontRegistry font = fontData[activeFontId];
    int activeWidth = Render_GetGlyphWidth(&font, activeAscii);
    int gridWidth = font.charWidth;
    if (gridWidth < 0) gridWidth = 0;
    if (gridWidth > 8) gridWidth = 8;
    int activeHeight = Render_GetGlyphHeight(&font);
    static const unsigned char zeroGlyph[FONT_VIEWER_MAX_GLYPH_ROWS] = { 0 };

    const unsigned char *sourceBytes = NULL;
    if (state->hasChanged[activeFontId][activeAscii]) {
        sourceBytes = state->customGlyphs[activeFontId][activeAscii];
    } else {
        sourceBytes = Render_FindGlyphBytes((unsigned int)activeAscii);
    }

    if (sourceBytes == NULL) {
        sourceBytes = zeroGlyph;
    }

    for (int py = 0; py < activeHeight; py++) {
        unsigned char row = sourceBytes[py];
        unsigned char mask = 0x01;

        for (int px = 0; px < gridWidth; px++) {
            int drawX = layout->previewRightAnchorX - (px * layout->previewBlockGap);
            int drawY = previewGridY + (py * layout->previewBlockGap);

            if (px >= activeWidth) {
                PR_RectFill(drawX, drawY, layout->previewBlockSize, layout->previewBlockSize, P8_DARK_GREY);
            } else if ((row & mask) > 0) {
                int paintColor = state->isEditMode ? P8_ORANGE : P8_YELLOW;
                PR_RectFill(drawX, drawY, layout->previewBlockSize, layout->previewBlockSize, paintColor);
            } else {
                PR_RectFill(drawX, drawY, layout->previewBlockSize, layout->previewBlockSize, P8_EXT_DARK_BLUE);
            }

            mask <<= 1;
        }
    }
}

static void Render_Legend(int x, int y) {
    PR_Print("Legend", x, y, P8_LIGHT_GREY);

    PR_RectFill(x, y + 9, 3, 3, P8_DARK_GREY);
    PR_Print("UNDEF", x + 6, y + 8, P8_LIGHT_GREY);

    PR_RectFill(x, y + 17, 3, 3, P8_YELLOW);
    PR_Print("SELECT", x + 6, y + 16, P8_LIGHT_GREY);

    PR_RectFill(x, y + 25, 3, 3, P8_ORANGE);
    PR_Print("EDIT", x + 6, y + 24, P8_LIGHT_GREY);
}

static void Render_BottomStatusBar(const FontViewerState *state) {
    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;
    int activeCode = Render_GetActiveAscii(state);

    PR_RectFill(0, 120, 128, 8, P8_RED);

    if (activeCode >= 0 && activeCode <= 255) {
        PR_Print(TextFormat("[%d][0x%02X][%s]", activeCode, activeCode, fontData[activeFontId].fontName), 2, 121, P8_DARK_PURPLE);
    } else {
        PR_Print(TextFormat("[--][--][%s]", fontData[activeFontId].fontName), 2, 121, P8_DARK_PURPLE);
    }
}

// void FontViewerRender_Draw(const FontViewerState *state, const FontViewerButtons *buttons, const FontViewerLayout *layout) {
void FontViewerRender_Draw(const FontViewerState *state, const FontViewerLayout *layout) {
    // if (state == NULL || buttons == NULL || layout == NULL) return;

    PR_Cls(P8_DARK_BLUE);

    int activeFontId = PR_GetActiveFontId();
    if (activeFontId < 0 || activeFontId >= FONT_COUNT) activeFontId = 0;

    FontRegistry font = fontData[activeFontId];
    int gridBoxH = Render_GetDynamicGridBoxHeight(state, layout, &font);
    int visibleRows = Render_GetVisibleGridRowsForBox(layout, &font, gridBoxH);
    int cellH = Render_GetGridCellHeight(&font);
    int cellW = layout->gridBoxW / 16;
    if (cellW < 1) cellW = 1;

    int lowerPanelY = layout->gridBoxY + gridBoxH + 1;
    if (lowerPanelY < 66) lowerPanelY = 66;
    int lowerPanelH = 120 - lowerPanelY;
    if (lowerPanelH < 1) lowerPanelH = 1;

    int previewGridY = lowerPanelY + 10;

    PR_RectFill(layout->gridBoxX, layout->gridBoxY, layout->gridBoxW, gridBoxH, P8_BLACK);
    PR_Line(layout->gridBoxX, layout->gridBoxY, layout->gridBoxX + layout->gridBoxW, layout->gridBoxY, P8_DARK_GREY);
    PR_Line(layout->gridBoxX, layout->gridBoxY + gridBoxH, layout->gridBoxX + layout->gridBoxW, layout->gridBoxY + gridBoxH, P8_DARK_GREY);

    PR_RectFill(0, lowerPanelY, 128, lowerPanelH, P8_BLACK);

    int totalRowsNeeded = Render_GetTotalRowsNeeded(state);
    int baseOffset = state->isFullMapMode ? 0 : 32;

    for (int y = 0; y < visibleRows; y++) {
        int actualRowIndex = state->scrollRowOffset + y;
        if (actualRowIndex >= totalRowsNeeded) break;

        for (int x = 0; x < 16; x++) {
            int targetAscii = baseOffset + (actualRowIndex * 16) + x;
            if (state->isFullMapMode && targetAscii > 255) break;
            if (!state->isFullMapMode && targetAscii > 126) break;

            int cx = layout->gridStartX + (x * cellW);
            int cy = layout->gridStartY + (y * cellH);

            int drawColor = P8_WHITE;
            if (state->hasChanged[activeFontId][targetAscii]) drawColor = P8_BLUE;
            if (targetAscii == state->hoveredAscii || targetAscii == state->selectedAscii) drawColor = P8_YELLOW;
            if (targetAscii == state->selectedAscii && state->isEditMode) drawColor = P8_ORANGE;

            if (targetAscii == state->hoveredAscii || targetAscii == state->selectedAscii) {
                PR_Rect(cx, cy, cellW, cellH, P8_DARK_GREY);
            }

            if (state->hasChanged[activeFontId][targetAscii]) {
                int activeWidth = Render_GetGlyphWidth(&font, targetAscii);
                int activeHeight = Render_GetGlyphHeight(&font);
                PR_Blit(state->customGlyphs[activeFontId][targetAscii], activeWidth, activeHeight, cx, cy, drawColor);
            } else {
                const unsigned char *glyphBytes = Render_FindGlyphBytes((unsigned int)targetAscii);
                if (glyphBytes != NULL) {
                    int activeWidth = Render_GetGlyphWidth(&font, targetAscii);
                    int activeHeight = Render_GetGlyphHeight(&font);
                    PR_Blit(glyphBytes, activeWidth, activeHeight, cx, cy, drawColor);
                } else {
                    PR_Print(".", (float)cx, (float)cy, P8_DARK_GREY);
                }
            }
        }
    }

    PR_Print(state->isEditMode ? "EDIT" : "VIEW", 42, lowerPanelY + 26, P8_LIGHT_GREY);
    PR_Print(state->isFullMapMode ? "FULL" : "PRINT", 42, lowerPanelY + 16, P8_BLUE);
    Render_Legend(78, lowerPanelY + 4);
    Render_MagnifiedFont(state, layout, previewGridY);

    //PR_DrawButton((Button *)&buttons->btnToggleMode);
    //PR_DrawButton((Button *)&buttons->btnEditMode);
    //PR_DrawButton((Button *)&buttons->btnPrintGlyph);

    Render_BottomStatusBar(state);
}
