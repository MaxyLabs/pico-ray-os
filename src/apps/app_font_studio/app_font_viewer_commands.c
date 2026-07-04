#include <stdio.h>
#include <string.h>
#include "app_font_viewer.h"

static int Commands_GetActiveFontId(void) {
    int fontId = PR_GetActiveFontId();
    if (fontId < 0 || fontId >= FONT_COUNT) {
        return 0;
    }
    return fontId;
}

static int Commands_ClampGlyphHeight(int charHeight) {
    if (charHeight < 0) return 0;
    if (charHeight > FONT_VIEWER_MAX_GLYPH_ROWS) return FONT_VIEWER_MAX_GLYPH_ROWS;
    return charHeight;
}

static int Commands_ClampGlyphWidth(int charWidth) {
    if (charWidth < 0) return 0;
    if (charWidth > 8) return 8;
    return charWidth;
}

static int Commands_GetGlyphWidthForCodepoint(const FontRegistry *font, int codepoint) {
    int width = (codepoint >= 32 && codepoint <= 126) ? font->printableWidth : font->charWidth;
    return Commands_ClampGlyphWidth(width);
}

static const unsigned char *Commands_FindGlyphBytes(unsigned int codepoint) {
    int activeFontId = PR_GetActiveFontId();
    
    // Pristine proxy link routing directly to our newly unified hardware dictionary engine pass
    return PR_GetFontGlyph(activeFontId, codepoint);
}

void FontViewerCommands_LockSelection(FontViewerState *state, int asciiCode) {
    if (state == NULL) return;
    if (asciiCode < 0 || asciiCode > 255) return;

    int fontId = Commands_GetActiveFontId();
    FontRegistry font = fontData[fontId];
    int glyphHeight = Commands_ClampGlyphHeight(font.charHeight);

    state->selectedAscii = asciiCode;
    memset(state->editBuffer, 0, sizeof(state->editBuffer));

    if (state->hasChanged[fontId][asciiCode]) {
        memcpy(state->editBuffer, state->customGlyphs[fontId][asciiCode], sizeof(state->editBuffer));
        return;
    }

    const unsigned char *liveBytes = Commands_FindGlyphBytes((unsigned int)asciiCode);
    if (liveBytes != NULL) {
        memcpy(state->editBuffer, liveBytes, (size_t)glyphHeight);
    }
}

void FontViewerCommands_UnlockSelection(FontViewerState *state) {
    if (state == NULL) return;

    state->selectedAscii = -1;
    state->isEditMode = false;
    memset(state->editBuffer, 0, sizeof(state->editBuffer));
}

void FontViewerCommands_ToggleEditMode(FontViewerState *state) {
    if (state == NULL) return;
    if (state->selectedAscii == -1) return;

    state->isEditMode = !state->isEditMode;
}

void FontViewerCommands_ToggleViewMode(FontViewerState *state) {
    if (state == NULL) return;

    state->isFullMapMode = !state->isFullMapMode;
    state->hoveredAscii = -1;
    state->scrollRowOffset = 0;
}

void FontViewerCommands_PrintGlyphToTerminal(const FontViewerState *state) {
    if (state == NULL) return;
    if (state->selectedAscii == -1) return;

    int fontId = Commands_GetActiveFontId();
    FontRegistry font = fontData[fontId];
    int glyphWidth = Commands_GetGlyphWidthForCodepoint(&font, state->selectedAscii);
    int glyphHeight = Commands_ClampGlyphHeight(font.charHeight);

    printf("\n>>> PICO-RAY FONT EDITOR | SINGLE GLYPH LIVE DUMP <<<\n");
    printf("Codepoint: %d | Symbol: '%c'\n\n", state->selectedAscii,
           (state->selectedAscii >= 32) ? (char)state->selectedAscii : ' ');

    printf("--- VISUAL RASTER RAMP ---\n");
    for (int y = 0; y < glyphHeight; y++) {
        unsigned char row = state->editBuffer[y];
        unsigned char mask = 0x01;
        printf("/* Line %d */ ", y);
        for (int x = 0; x < glyphWidth; x++) {
            putchar(((row & mask) > 0) ? 'X' : '.');
            mask <<= 1;
        }
        putchar('\n');
    }

    printf("\n--- C-COMPILER DATA CODE STRIP ---\n");
    printf("    { %d, { ", state->selectedAscii);
    for (int y = 0; y < glyphHeight; y++) {
        printf("0x%02X", state->editBuffer[y]);
        if (y < glyphHeight - 1) {
            printf(", ");
        }
    }
    printf(" } }, // '%c' (Codepoint %d)\n",
           (state->selectedAscii >= 32) ? (char)state->selectedAscii : ' ', state->selectedAscii);
    printf(">>> END OF SINGLE GLYPH LIVE DUMP <<<\n\n");
}

void FontViewerCommands_ExportAllModifiedGlyphs(const FontViewerState *state) {
    if (state == NULL) return;

    int fontId = Commands_GetActiveFontId();
    FontRegistry font = fontData[fontId];
    int glyphHeight = Commands_ClampGlyphHeight(font.charHeight);

    char fileTargetName[64];
    snprintf(fileTargetName, sizeof(fileTargetName), "carts/font-%d.txt", fontId);

    FILE *file = fopen(fileTargetName, "w");
    if (!file) {
        printf("PICO-RAY OS | FONT STUDIO ERROR: Cannot construct consolidated export disk target!\n");
        return;
    }

    fprintf(file, "========================================================================\n");
    fprintf(file, "PICO-RAY OS | FONT STUDIO REPORT PACK | TYPEFACE PACK ID: %d\n", fontId);
    fprintf(file, "========================================================================\n\n");

    int alteredCount = 0;

    fprintf(file, "=== BLOCK 1: VISUAL CHARACTER RASTER CATALOGUE ===\n");
    fprintf(file, "------------------------------------------------------------------------\n");
    for (int c = 0; c < 256; c++) {
        if (!state->hasChanged[fontId][c]) continue;

        int glyphWidth = Commands_GetGlyphWidthForCodepoint(&font, c);

        alteredCount++;
        fprintf(file, "/// GLYPH CODEPOINT RECORD: %d (Symbol: '%c') ///\n", c, (c >= 32) ? (char)c : ' ');

        for (int y = 0; y < glyphHeight; y++) {
            unsigned char row = state->customGlyphs[fontId][c][y];
            unsigned char mask = 0x01;

            fprintf(file, "/* Line %d */ \"", y);
            for (int x = 0; x < glyphWidth; x++) {
                fputc(((row & mask) > 0) ? 'X' : '.', file);
                mask <<= 1;
            }
            fprintf(file, "\"\n");
        }
        fprintf(file, "------------------------------------------------------------------------\n");
    }
    fprintf(file, "\n");

    fprintf(file, "=== BLOCK 2: C-ARRAY CONFIGURATION BLOCK (READY FOR data-fonts.c) ===\n");
    fprintf(file, "------------------------------------------------------------------------\n");
    for (int c = 0; c < 256; c++) {
        if (!state->hasChanged[fontId][c]) continue;

        int glyphWidth = Commands_GetGlyphWidthForCodepoint(&font, c);

        fprintf(file, "    { %d, { ", c);
        for (int y = 0; y < glyphHeight; y++) {
            fprintf(file, "0x%02X", state->customGlyphs[fontId][c][y]);
            if (y < glyphHeight - 1) {
                fprintf(file, ", ");
            }
        }
        fprintf(file, " } }, // '%c' (Codepoint %d)\n", (c >= 32) ? (char)c : ' ', c);
    }
    fprintf(file, "------------------------------------------------------------------------\n");
    fprintf(file, "/// END OF EXPORT BUNDLE. TOTAL ALTERED CHARACTER RECORDS PACKED: %d ///\n", alteredCount);

    fclose(file);
    printf("PICO-RAY OS | FONT STUDIO: Two-block dynamic session export successful! Saved %d records straight to %s\n",
           alteredCount, fileTargetName);
}
