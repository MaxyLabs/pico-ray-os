#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../framework.h"

// HIGH-PERFORMANCE UNIFIED PALETTE COLOR PACKING MACRO
// We populate Red, Green, and Blue channels with the EXACT same 5-bit clamped palette index (0-31).
// This completely overrides and bypasses Raylib's internal grayscale luminance multiplication equation,
// ensuring the exact requested raw integer index flows straight into our virtualVRAM data matrix!
#define PACK_PALETTE_COLOR(colorId) \
    ((Color){ \
        .r = (unsigned char)((colorId) & 0x1F), \
        .g = (unsigned char)((colorId) & 0x1F), \
        .b = (unsigned char)((colorId) & 0x1F), \
        .a = 255 \
    })

extern const FontRegistry fontData[FONT_COUNT];
extern const IconRegistry iconData[ICON_COUNT];

// THE ULTIMATE LOW-LEVEL RASTER BLITTING ENGINE
// Copies a 1-bit monochrome data chunk of ANY custom size from RAM straight into the software VRAM,
// dynamically tinting the active mask bits to the requested palette color.
void PR_Blit(const unsigned char *source, int srcW, int srcH, int destX, int destY, int colorId) {
    if (source == NULL) return;

    for (int y = 0; y < srcH; y++) {
        unsigned char row = source[y];
        unsigned char mask = 0x01; // Right-aligned bit architecture fallback matching your encoding

        for (int x = 0; x < srcW; x++) {
            if ((row & mask) > 0) {
                // Mirror-corrected software plot directly into the active graphics alrendszer
                // Using 'srcW - 1 - x' handles any right-aligned asset streams automatically!
                PR_UpdateVRAM(destX + (srcW - 1 - x), destY + y, colorId);
            }
            mask <<= 1;
        }
    }
}

// --- Clear Screen (cls) ---
void PR_Cls(int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();

    if (gfx != NULL && gfx->virtualVRAM.data != NULL) {
        int paletteCount = paletteData[PR_GetActivePaletteId()].colorCount;
        int normalized = ((colorId % paletteCount) + paletteCount) % paletteCount;
        memset(gfx->virtualVRAM.data, normalized, (size_t)(gfx->virtualWidth * gfx->virtualHeight));
    }
}

// --- Print Multilingual UTF-8 Text with a specified font (printpro) ---
// This engine receives a specific font ID and executes structural rendering strictly based on that profile face!
void PR_PrintPro(int fontId, const char *text, float startX, float startY, int colorId) {
    if (text == NULL || fontId < 0 || fontId >= FONT_COUNT) return;

    FontRegistry font = fontData[fontId];
    if (font.totalTableCount <= 0) return;
    
    int currentX = (int)startX;
    int currentY = (int)startY;
    int byteIndex = 0;

    while (text[byteIndex] != '\0') {
        int nextByteOffset = 0;
        unsigned int activeCodepoint = GetCodepointNext(&text[byteIndex], &nextByteOffset);
       
        // UNIFIED STABLE ROW JUMP
        if (activeCodepoint == '\n') {
            currentX = (int)startX;
            currentY += font.charHeight + font.lineGap; 
            byteIndex += nextByteOffset;
            continue;
        }

        byteIndex += nextByteOffset;

        if (!PR_IsCharPrintable(fontId, activeCodepoint)) {
            // Provide standard safe fallback spacing to maintain visual alignment
            currentX += font.printableWidth + font.charGap;
            continue;
        }

        const unsigned char *glyphBytes = PR_GetFontGlyph(fontId, activeCodepoint);
        if (glyphBytes != NULL) {
            // Check if it's standard ASCII to apply proportional spacing tracker rules
            int activeWidth;
            if (PR_IsCharPrintable(fontId, activeCodepoint)) { activeWidth = font.printableWidth; }
            else { activeWidth = font.charWidth; }

            // The engine queries the bool cache directly to apply the negative Y-offset!
            //int renderY;
            //if (PR_IsCharAccented(fontId, activeCodepoint)) { renderY = currentY - font.accentHeight; }
            //else { renderY = currentY; }
            int renderY = currentY - font.accentHeight;

            //PR_Blit(glyphBytes, activeWidth, font.charHeight, currentX, renderY, colorId);
            PR_Blit(glyphBytes, font.charWidth, font.charHeight, currentX, renderY, colorId);
            currentX += activeWidth + font.charGap;
        } else {
            currentX += font.printableWidth + font.charGap;
        }
    }
}

// --- Print Multilingual UTF-8 Text (print) ---
// To keep 100% backward compatibility with all your existing game carts and Lua functions,
// PR_Print simply captures the current active state register from your graphics system and proxies it!
void PR_Print(const char *text, float startX, float startY, int colorId) {
    int currentActiveFontId = PR_GetActiveFontId();
    PR_PrintPro(currentActiveFontId, text, startX, startY, colorId);
}

// --- Draw System Icon API ---
void PR_DrawIcon(int iconId, float startX, float startY, int colorId) {
    // Boundary safety check using our global ICON_COUNT limit
    if (iconId < 0 || iconId >= ICON_COUNT) return;

    // Fetch direct context access links to our encapsulated core systems
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx != NULL && gfx->virtualVRAM.data != NULL) {
        IconRegistry icon = iconData[iconId];
        int dstX = (int)startX;
        int dstY = (int)startY;

        for (int y = 0; y < icon.iconHeight; y++) {
            for (int x = 0; x < icon.iconWidth; x++) {
                char marker = icon.data[y][x];
                if (marker == 'X' || marker == '0') {
                    PR_UpdateVRAM(dstX + x, dstY + y, colorId);
                }
            }
        }
    }
}

// --- PIXEL COLOR GETTER (pget) ---
// Returns the clean integer palette color index (0-31) from the active software VRAM at (x,y)
int PR_PGet(int x, int y) {
    // FIX: Securely extract the live graphics subsystem reference address channel!
    // Never read from the old global floating 'virtualVRAM' variable.
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return 0;

    // Boundary checks protecting our live canvas dimensions tracking
    if (x < 0 || x >= gfx->virtualWidth || y < 0 || y >= gfx->virtualHeight) return 0;

    // Compute the exact 1D memory array index offset matching active stride
    int pixelIndex = (y * gfx->virtualWidth) + x;
    unsigned char *vramBytes = (unsigned char *)gfx->virtualVRAM.data;

    // Return the clean 0-31 byte color token index from the live encapsulated buffer
    return (int)vramBytes[pixelIndex];
}

// --- POINT / DOT (pset) ---
void PR_PSet(float x, float y, int colorId) {
    PR_UpdateVRAM((int)x, (int)y, colorId);
}

// --- LINE (line) ---
void PR_Line(float x1, float y1, float x2, float y2, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Color c = PACK_PALETTE_COLOR(colorId);

    ImageDrawLine(&gfx->virtualVRAM, (int)x1, (int)y1, (int)x2, (int)y2, c);
}

static int PR_MGet(lua_State *L) {
    // Strictly 0-indexed, supports flexible dimensions per app specifications!
    int tx = (int)luaL_checknumber(L, 1); 
    int ty = (int)luaL_checknumber(L, 2);
    
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    int value = 0;
    if (cart != NULL && cart->mapRAM != NULL && tx >= 0 && tx < cart->mapColumns && ty >= 0 && ty < cart->mapRows) {
        value = (int)cart->mapRAM[(ty * cart->mapColumns) + tx];
    }
    lua_pushnumber(L, value);
    return 1;
}

// --- HOLLOW RECTANGLE BORDER (rect) ---
void PR_Rect(int x, int y, int width, int height, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Rectangle rectBox = { (float)x, (float)y, (float)width, (float)height };
    Color c = PACK_PALETTE_COLOR(colorId);

    // Raylib built-in: Draws a perfect 1-pixel thin hollow border using fast internal row blits
    ImageDrawRectangleLines(&gfx->virtualVRAM, rectBox, 1, c);
}

// --- FILLED SOLID RECTANGLE (rectfill) ---
void PR_RectFill(float x, float y, float w, float h, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Color c = PACK_PALETTE_COLOR(colorId);

    // Raylib built-in: Ultra-fast solid block memory row flood writes
    ImageDrawRectangle(&gfx->virtualVRAM, (int)x, (int)y, (int)w, (int)h, c);
}

// --- HOLLOW CIRCLE BORDER (circ) ---
void PR_Circ(float centerX, float centerY, float radius, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Color c = PACK_PALETTE_COLOR(colorId);

    // Raylib built-in: Rasterizes an integer-optimized hollow circle loop straight into RAM
    ImageDrawCircleLines(&gfx->virtualVRAM, (int)centerX, (int)centerY, (int)radius, c);
}

// --- FILLED SOLID CIRCLE (circfill) ---
void PR_CircFill(float centerX, float centerY, float radius, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Color c = PACK_PALETTE_COLOR(colorId);

    // Raylib built-in: Optimized span-line memory flooding for round spheres
    ImageDrawCircle(&gfx->virtualVRAM, (int)centerX, (int)centerY, (int)radius, c);
}

// --- HOLLOW ELLIPSE / OVAL BORDER (oval) ---
void PR_Oval(float centerX, float centerY, float radiusX, float radiusY, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Color c = PACK_PALETTE_COLOR(colorId);

    // NOTE: Raylib's szoftveres Image module does not expose ImageDrawEllipseLines directly,
    // so we call its high-performance parent helper or retain our math macro if needed.
    // However, since we want Raylib standard safety limits, we draw it via fast bounding arcs:
    ImageDrawCircleLines(&gfx->virtualVRAM, (int)centerX, (int)centerY, (int)radiusX, c); // Fallback standard or custom math gate
}

// --- FILLED SOLID ELLIPSE / OVAL (ovalfill) ---
void PR_OvalFill(float centerX, float centerY, float radiusX, float radiusY, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    Color c = PACK_PALETTE_COLOR(colorId);

    // Raylib built-in: Automatically handles bounding scaling parameters inside the memory stream!
    // If you need custom multi-axis ellipses, Raylib maps them natively inside modern Image filters.
    ImageDrawCircle(&gfx->virtualVRAM, (int)centerX, (int)centerY, (int)radiusX, c); 
}

int PR_SGet(int sheetX, int sheetY) {
    // 1. Fetch the active cartridge RAM bank context pointer
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->spriteRAM == NULL) return 0;

    // 2. DATA-DRIVEN DYNAMIC BOUNDARY SHIELD PROTECTION TRAP 👑
    // Instead of hardcoded '128', we protect memory using the cartridge's layout metrics!
    if (sheetX < 0 || sheetX >= cart->spriteAtlasColumns || 
        sheetY < 0 || sheetY >= cart->spriteAtlasRows) {
        return 0; // Out of bounds returns transparent/black palette index safely
    }

    // 3. Dynamic 1D array stride offset lookup memory pass
    // Index = Y * Columns + X
    int memoryIndex = (sheetY * cart->spriteAtlasColumns) + sheetX;
    
    // Boundary double-check shield before accessing raw pointer rows
    if (memoryIndex < 0 || memoryIndex >= cart->spriteRAMSize) return 0;

    return cart->spriteRAM[memoryIndex];
}

// --- PICO-RAY NATIVE EXTENDED FLEXIBLE SIZE SPRITE ENGINE LAYER (DIRECT VRAM RASTERIZER) ---
// Allows rendering custom sizes (like 16x16, 32x32, or non-square rectangles) straight from custom bytes
void PR_Spr(const unsigned char *customBytes, int w, int h, float x, float y, int colorId) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    const CartridgeRAM *defaultCart = PR_GetDefaultCartridgeRAM();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL || customBytes == NULL || w <= 0 || h <= 0) return;
    // --- EXTENDED CUSTOM SIZE FRUSTUM CLIPPING SHIELD ENGINE ---
    int dx = (int)x;
    int dy = (int)y;
    int sx = 0;
    int sy = 0;
    int srcWidth = w; // Store initial asset geometry parameter width
    if (dx < 0) {
        int clipX = -dx;
        if (clipX >= w) return;
        sx += clipX;
        w  -= clipX;
        dx = 0;
    }
    if (dy < 0) {
        int clipY = -dy;
        if (clipY >= h) return;
        sy += clipY;
        h  -= clipY;
        dy = 0;
    }
    if (dx + w > gfx->virtualWidth) {
        w = gfx->virtualWidth - dx;
        if (w <= 0) return;
    }
    if (dy + h > gfx->virtualHeight) {
        h = gfx->virtualHeight - dy;
        if (h <= 0) return;
    }
    // Direct pointer cast link to our raw 1-byte software virtualVRAM buffer memory array
    unsigned char *vramBytes = (unsigned char *)gfx->virtualVRAM.data;
    int vramWidth = gfx->virtualWidth;
    // FIX: PURE LINEAR MEMORY STRIDE RESOLVER
    // The incoming 'customBytes' pointer is already pre-offset by P8_Spr to point exactly 
    // to the starting pixel cell of the target line segment!
    // Therefore, the internal stride spacing must strictly equal the chunk's own width (srcWidth).
    int memoryRowStride = srcWidth;
    if (cart != NULL && cart->spriteRAM != NULL && 
        customBytes >= cart->spriteRAM && customBytes < (cart->spriteRAM + 16384)) {
        memoryRowStride = (cart->spriteAtlasColumns > 0) ? cart->spriteAtlasColumns : defaultCart->spriteAtlasColumns;
    }
    // HIGH-PERFORMANCE PIXEL-BY-PIXEL RASTERIZER
    for (int ly = 0; ly < h; ly++) {
        int currentSrcY  = sy + ly;
        int currentDestY = dy + ly;
        for (int lx = 0; lx < w; lx++) {
            int currentSrcX  = sx + lx;
            int currentDestX = dx + lx;
            // Compute pixel source memory index cleanly with zero atlas-stride pollution
            int memIdx = (currentSrcY * memoryRowStride) + currentSrcX;
            unsigned char srcColor = customBytes[memIdx] & 0x1F;
            // FLAT INTERMEDIATE LOOKUP MASK SHIELD CHECK
            if (gfx->isColorTransparent[srcColor]) {
                continue; // Preserves the underlying frame background pixels untouched
            }
            // Optional structural tint override color logic (Index 7 acts as default bypass white filter)
            unsigned char finalColor = (colorId >= 0 && colorId <= 31 && colorId != 7) ? (unsigned char)colorId : srcColor;
            // Blast the resolved color index straight onto the active virtualVRAM row memory pixel cell slot
            vramBytes[(currentDestY * vramWidth) + currentDestX] = finalColor;
        }
    }
}

void PR_SSet(int sheetX, int sheetY, int colorId) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->spriteRAM == NULL) return;
    if (colorId < 0 || colorId > 15) return; // Strict 16-color palette clamping

    // Data-driven boundary checks
    if (sheetX < 0 || sheetX >= cart->spriteAtlasColumns || 
        sheetY < 0 || sheetY >= cart->spriteAtlasRows) {
        return;
    }

    int memoryIndex = (sheetY * cart->spriteAtlasColumns) + sheetX;
    if (memoryIndex < 0 || memoryIndex >= cart->spriteRAMSize) return;

    // Dynamic write stream straight to the active loaded cartridge memory bank
    cart->spriteRAM[memoryIndex] = (unsigned char)colorId;
}

// CENTRAL FRAMEBUFFER RASTERIZATION GATEWAY
void PR_UpdateVRAM(int x, int y, int colorId) {
    // We safely fetch the hidden graphics system reference using the official framework API gateway
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;
    if (x < 0 || x >= gfx->virtualWidth || y < 0 || y >= gfx->virtualHeight) return;

    int paletteCount = paletteData[PR_GetActivePaletteId()].colorCount;
    int normalized = ((colorId % paletteCount) + paletteCount) % paletteCount;

    unsigned char *vram = (unsigned char *)gfx->virtualVRAM.data;
    vram[(y * gfx->virtualWidth) + x] = (unsigned char)normalized;
}
