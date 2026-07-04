// --- Inside src/framework/modules/data-font-glyphs.h ---
#ifndef DATA_FONT_GLYPH_H
#define DATA_FONT_GLYPH_H

#define GLYPHMAP_PICORAY_COUNT          95  // Put your exact count here!
#define GLYPHMAP_PICO8_COUNT           256  // Put your exact count here!
#define GLYPHMAP_PICORAY_UTF8_HU_COUNT  14  // Put your exact count here!
#define GLYPHMAP_PICORAY_UTF8_CE_COUNT  28
#define GLYPHMAP_PICORAY_UTF8_WE_COUNT  18
#define GLYPHMAP_PICORAY_UTF8_NE_COUNT  13
#define GLYPHMAP_PICORAY_UTF8_SE_COUNT  12
#define GLYPHMAP_QUARTZ_COUNT           122


// #define ARRAY_LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))

#include "../../framework.h" // Ensures the GlyphMapping struct type is known

// UNIFIED EXPORT MODULE BRIDGES
// Centralized catalog of all available European text engine shard matrices.
extern const GlyphMapping GLYPHMAP_PICORAY[];

extern const GlyphMapping GLYPHMAP_PICORAY_UTF8_HU[];
extern const GlyphMapping GLYPHMAP_PICORAY_UTF8_CE[];
extern const GlyphMapping GLYPHMAP_PICORAY_UTF8_WE[];
extern const GlyphMapping GLYPHMAP_PICORAY_UTF8_NE[];
extern const GlyphMapping GLYPHMAP_PICORAY_UTF8_SE[];

extern const GlyphMapping GLYPHMAP_PICO8[];

extern const GlyphMapping GLYPHMAP_QUARTZ[];

// extern const int GLYPHMAP_PICORAY_COUNT;
// extern const int GLYPHMAP_PICORAY_UTF8_HU_COUNT;
// extern const int GLYPHMAP_PICORAY_UTF8_CE_COUNT;
// extern const int GLYPHMAP_PICORAY_UTF8_WE_COUNT;
// extern const int GLYPHMAP_PICORAY_UTF8_NE_COUNT;
// extern const int GLYPHMAP_PICORAY_UTF8_SE_COUNT;
// extern const int GLYPHMAP_PICO8_COUNT;
// extern const int GLYPHMAP_QUARTZ_COUNT;

#endif // DATA_FONT_GLYPH_H
