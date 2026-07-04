#include <string.h>
#include "../framework.h"

static FontPreloadCache fontCache[FONT_COUNT];

void PR_InitFontCache(void) {
    // Clear out cache memory to safe false states
    memset(fontCache, 0, sizeof(fontCache));

    for (int f = 0; f < FONT_COUNT; f++) {
        FontRegistry font = fontData[f];
        if (font.totalTableCount <= 0) continue;

        for (unsigned int cp = 0; cp < MAX_FONT_CACHE_CODEPOINTS; cp++) {
            // 1. Evaluate Printability
            bool printableStatus = (cp >= 32 && cp <= 126);
            
            const unsigned char *glyph = PR_GetFontGlyph(f, cp);
            if (glyph != NULL) {
                printableStatus = true;
            }

            // DIRECTLY ASSIGN THE BOOL STATE 👑
            fontCache[f].isPrintable[cp] = printableStatus;

            // 2. Evaluate Accent Overlay metrics
            if (font.accentHeight > 0 && cp > 126 && glyph != NULL) {
                fontCache[f].isAccented[cp] = true;
            }
        }
    }
    printf("PICO-RAY | KERNEL | Font array-cache initialized smoothly for %d profiles!\n", FONT_COUNT);
}

bool PR_IsCharPrintable(int fontId, unsigned int codepoint) {
    if (fontId < 0 || fontId >= FONT_COUNT || codepoint >= MAX_FONT_CACHE_CODEPOINTS) return false;
    return fontCache[fontId].isPrintable[codepoint]; // No bit shifts, just pure direct RAM query!
}

bool PR_IsCharAccented(int fontId, unsigned int codepoint) {
    if (fontId < 0 || fontId >= FONT_COUNT || codepoint >= MAX_FONT_CACHE_CODEPOINTS) return false;
    return fontCache[fontId].isAccented[codepoint];
}

const unsigned char* PR_GetFontGlyph(int fontId, unsigned int codepoint) {
    if (fontId < 0 || fontId >= FONT_COUNT) return NULL;

    FontRegistry font = fontData[fontId];
    if (font.totalTableCount <= 0) return NULL;

    // Multi-table segment scanner loop
    for (int t = 0; t < font.totalTableCount; t++) {
        const GlyphMapping *currentTable = font.glyphTables[t];
        int currentCount = font.glyphCounts[t];
        
        if (currentTable == NULL || currentCount <= 0) continue;

        for (int i = 0; i < currentCount; i++) {
            if (currentTable[i].codePoint == codepoint) {
                return currentTable[i].bytes; 
            }
        }
    }

    // Default safety fallback mapping to avoid NULL reference execution traps
    if (font.totalTableCount > 0 && font.glyphTables[0] != NULL) {
        return font.glyphTables[0]->bytes;
    }

    return NULL;
}

// Iterates through UTF-8 sequences and sums up character widths plus tracking spaces cleanly.
int PR_GetStringWidthByPixel(int fontId, const char *text) {
    if (text == NULL || fontId < 0 || fontId >= FONT_COUNT) return 0;

    FontRegistry font = fontData[fontId];
    if (font.totalTableCount <= 0) return 0;

    int maxWidth = 0;
    int currentLineWidth = 0;
    int byteIndex = 0;

    while (text[byteIndex] != '\0') {
        int nextByteOffset = 0;
        unsigned int activeCodepoint = GetCodepointNext(&text[byteIndex], &nextByteOffset);

        // MULTI-LINE NEWLINE SUPPORT
        // If a newline token is hit, benchmark the current row line width and reset counters
        if (activeCodepoint == '\n') {
            if (currentLineWidth > maxWidth) {
                maxWidth = currentLineWidth;
            }
            currentLineWidth = 0;
            byteIndex += nextByteOffset;
            continue;
        }

        byteIndex += nextByteOffset;

        // Query character width: standard ASCII takes printableWidth, higher UTF-8 takes charWidth
        int activeWidth = (activeCodepoint >= 32 && activeCodepoint <= 126) ? font.printableWidth : font.charWidth;

        // Add character width + 1 pixel hardware letter tracking space gap
        currentLineWidth += activeWidth + font.charGap;
    }

    // Final trailing line layout benchmark check
    if (currentLineWidth > maxWidth) {
        maxWidth = currentLineWidth;
    }

    // Remove the very last trailing charGap padding from the final output length score
    return (maxWidth > 0) ? (maxWidth - font.charGap) : 0;
}
