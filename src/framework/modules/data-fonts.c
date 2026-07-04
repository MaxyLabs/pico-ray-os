#include "../framework.h"
#include "data-font-glyphs/data-font-glyphs.h"

// --- EXTENSION GLYPH PACKAGES ---
// TODO: const unsigned char HUNGARIAN_ACCENTS[] = { /* Unicode 0x00C0 - 0x017F extension row bytes */ };
// TODO: const unsigned char JAPANESE_KATAKANA[]  = { /* Unicode 0x30A0 - 0x30FF row bytes */ };

// --- ULTRA COMPACT 3x5 / 4x5 RETRO TINY GLYPHS ---
// Tailored micro resolution grids designed to maximize data density in 128x128 views
const unsigned char TINY_GLYPHS[1][6] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } // Space
};

// --- CENTRAL HARDWARE TYPOGRAPHY REGISTRY ---
// You can structure them as one large master table, or use this dictionary style layout
const FontRegistry fontData[FONT_COUNT] = {
    [FONT_PICORAY] = {
        .fontName = "PICORAY",
        .charWidth = 8,
        .charHeight = 8,
        .printableWidth = 4,
        .accentHeight = 2,
        .lineGap = 2,
        .charGap = 0,
        .glyphTables = { GLYPHMAP_PICORAY },
        .glyphCounts = { GLYPHMAP_PICORAY_COUNT },
        .totalTableCount = 1
    },
    [FONT_PICO8] = {
        .fontName = "PICO8",
        .charWidth = 8,
        .charHeight = 6,
        .printableWidth = 4,
        .glyphTables = { GLYPHMAP_PICO8 },
        .glyphCounts = { GLYPHMAP_PICO8_COUNT },
        .totalTableCount = 1
    },
    [FONT_PICORAY_UTF8] = {
        .fontName = "PICORAY-UTF8-EU",
        .charWidth = 8,
        .charHeight = 8,
        .accentHeight = 2,
        .printableWidth = 4,
        
        // FIX 2: THE MONOLITHIC PAN-EUROPEAN MULTI-TABLE SHIFT MATRIX 🥪
        // Order follows exact baseline and padding hierarchy layout steps cleanly!
        .glyphTables = { 
            GLYPHMAP_PICORAY,     // Index 0: Baseline English (6px tall data)
            GLYPHMAP_PICORAY_UTF8_HU,   // Index 1: Hungarian Accents (8px tall data)
            GLYPHMAP_PICORAY_UTF8_CE,   // Index 2: Central Europe (Czech/Polish)
            GLYPHMAP_PICORAY_UTF8_WE,   // Index 3: Western Europe (French/Dutch)
            GLYPHMAP_PICORAY_UTF8_NE,   // Index 4: Northern Europe (German/Nordic)
            GLYPHMAP_PICORAY_UTF8_SE    // Index 5: Southern Europe (Spanish/Italian)
        },
        .glyphCounts = { 
            GLYPHMAP_PICORAY_COUNT,
            GLYPHMAP_PICORAY_UTF8_HU_COUNT,
            GLYPHMAP_PICORAY_UTF8_CE_COUNT,
            GLYPHMAP_PICORAY_UTF8_WE_COUNT,
            GLYPHMAP_PICORAY_UTF8_NE_COUNT,
            GLYPHMAP_PICORAY_UTF8_SE_COUNT
        },
        .totalTableCount = 6 // 6 master structures combined into one single logical font!
    },
    [FONT_QUARTZ] = {
        .fontName = "QUARTZ",
        .charWidth = 8,
        .charHeight = 8,
        .printableWidth = 8,
        .accentHeight = 0,
        
        // FIX 2: THE MONOLITHIC PAN-EUROPEAN MULTI-TABLE SHIFT MATRIX 🥪
        // Order follows exact baseline and padding hierarchy layout steps cleanly!
        .glyphTables = { 
            GLYPHMAP_QUARTZ        // Index 0: Baseline English 
         },
        .glyphCounts = { 
            GLYPHMAP_QUARTZ_COUNT
        },
        .totalTableCount = 1 // 6 master structures combined into one single logical font!
    }
};
