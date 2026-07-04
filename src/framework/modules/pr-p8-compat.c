#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../framework.h"

// --- PICO-8 COMPATIBLE GRAPHICS ENGINE LAYER (DYNAMIC RESOLUTION ALIGNED) ---
void P8_Spr(int spriteId, float destX, float destY, float w, float h, bool flipX, bool flipY) {
    if (spriteId < 0 || w <= 0.0f || h <= 0.0f) return;

    CartridgeRAM *cart = PR_GetCartridgeRAM();
    const CartridgeRAM *defaultCart = PR_GetDefaultCartridgeRAM();

    if (cart == NULL || cart->spriteRAM == NULL) return;

    // FIX 1: DYNAMIC SPRITES-PER-ROW CALCULATION
    // Instead of forcing a hardcoded 16-stride loop divider (which belongs to a strict 128px sheet width format),
    // we derive the real sprites-per-row capacity directly from our newly configured dynamic spriteAtlasColumns register!
    int sheetStride = (cart->spriteAtlasColumns > 0) ? cart->spriteAtlasColumns : defaultCart->spriteAtlasColumns;
    int spritesPerRow = sheetStride / 8;
    if (spritesPerRow <= 0) spritesPerRow = 16; // Extreme safety fallback fence guard rails

    // Convert PICO-8 sprite sorszám identifier index directly into dynamic sheet pixel anchors
    int srcX = (spriteId % spritesPerRow) * 8;
    int srcY = (spriteId / spritesPerRow) * 8;

    // Translate float sprite unit scales directly to exact pixel target boundaries
    float pixelW = w * 8.0f;
    float pixelH = h * 8.0f;

    // 2. HARDWARE VERTICAL ROW-BY-ROW RASTER SLICER ENGINE
    for (int lineY = 0; lineY < (int)pixelH; lineY++) {
        // Apply vertical flip_y pixel addressing offset invert logic if requested by the software layer
        int targetSrcLineY = flipY ? ((int)pixelH - 1 - lineY) : lineY;
        int currentSheetY  = srcY + targetSrcLineY;

        // Matrix boundary safety clipping guard rails
        if (currentSheetY < 0) continue;

        if (flipX) {
            // If horizontal flip is active, we blit pixel-by-pixel inverted left-to-right.
            // We pass a width of 1 pixel per block, letting PR_Spr execute transparency masks on it.
            for (int lineX = 0; lineX < (int)pixelW; lineX++) {
                int targetSrcX = srcX + ((int)pixelW - 1 - lineX);
                if (targetSrcX < 0 || targetSrcX >= sheetStride) continue;

                const unsigned char *pixelByte = &cart->spriteRAM[(currentSheetY * sheetStride) + targetSrcX];
                
                // Pass 7 as default tint code instead of 0 to protect pixel values integrity
                PR_Spr(pixelByte, 1, 1, destX + lineX, destY + lineY, 7); 
            }
        } else {
            // FAST HIGH-SPEED EXECUTION CHANNEL!
            // We can now blit the entire horizontal strip slice safely in one call!
            // Our freshly updated PR_Spr rasters will automatically step pixel-by-pixel 
            // inside this row segment evaluating transparency masks dynamically.
            if (srcX < 0 || srcX >= sheetStride) continue;
            
            const unsigned char *rowPointerBytes = &cart->spriteRAM[(currentSheetY * sheetStride) + srcX];
            PR_Spr(rowPointerBytes, (int)pixelW, 1, destX, destY + lineY, 7);
        }
    }
}

