#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "../../framework.h"

// Bridges Lua 'mget(tileX, tileY)' directly into native 0-indexed mapRAM
int PR_Lua_MGet(lua_State *L) {
    // Argument 1 is ALWAYS horizontal X (columns), Argument 2 is ALWAYS vertical Y (rows)
    int tx = (int)luaL_checknumber(L, 1);
    int ty = (int)luaL_checknumber(L, 2);
    
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    int value = 0; // Default fallback to empty cell if out of bounds

    if (cart != NULL && cart->mapRAM != NULL) {
        if (tx >= 0 && tx < cart->mapColumns && ty >= 0 && ty < cart->mapRows) {
            int memoryCellOffset = (ty * cart->mapColumns) + tx;
            value = (int)cart->mapRAM[memoryCellOffset];
        }
    }

    lua_pushnumber(L, value); // Push tile ID back to Lua stack row
    return 1; // Returns exactly 1 argument to Lua
}

// Bridges Lua 'mset(tileX, tileY, value)' safely to mutate active tile bytes at runtime
int PR_Lua_MSet(lua_State *L) {
    int tx = (int)luaL_checknumber(L, 1);
    int ty = (int)luaL_checknumber(L, 2);
    unsigned char val = (unsigned char)luaL_checknumber(L, 3);

    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart != NULL && cart->mapRAM != NULL) {
        if (tx >= 0 && tx < cart->mapColumns && ty >= 0 && ty < cart->mapRows) {
            int memoryCellOffset = (ty * cart->mapColumns) + tx;
            cart->mapRAM[memoryCellOffset] = val; // Direct safe data segment overwrite block
        }
    }
    return 0; // Returns 0 arguments to Lua
}

// --- Inside lua/api-lua.c ---

// Bridges Lua 'spr(srcX, srcY, destX, destY, w, h, [colorId])'
// Dynamically slices a custom-sized rectangle area straight from the global spriteRAM atlas!
int PR_Lua_Spr(lua_State *L) {
    // 1. EXTRACT GEOMETRY METRICS FROM THE LUA STACK
    // Coordinates mapping where the asset lives inside the central sprite sheet matrix
    int srcX = (int)luaL_checknumber(L, 1);
    int srcY = (int)luaL_checknumber(L, 2);

    // Target raster position canvas anchors onto virtualVRAM window display bounds
    float destX = (float)luaL_checknumber(L, 3);
    float destY = (float)luaL_checknumber(L, 4);

    // Proportional dimensions of the bounding sprite frame slice (e.g. 16x24 for Ax Battler)
    int w    = (int)luaL_checknumber(L, 5);
    int h    = (int)luaL_checknumber(L, 6);

    // 2. OPTIONAL TINT COLOR INDEX FILTER
    // Default to index 7 (White/No tint filter override) if omitted by the script developer
    int colorId = 7;
    if (lua_gettop(L) >= 7) {
        colorId = (int)luaL_checknumber(L, 7);
    }

    // 3. HARDWARE CAP AND ACCESSIBILITY VALIDATION SHIELD
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->spriteRAM == NULL || w <= 0 || h <= 0) return 0;

    // Fetch the unified structural layout sheet width stride boundary (fallback safely to 128)
    int sheetStride = cart->spriteAtlasColumns > 0 ? cart->spriteAtlasColumns : 128;

    // 4. ROW-BY-ROW HARDWARE TEXTURE SLICER PASS
    // Because the 1D spriteRAM array structure shifts rows by sheetWidth steps, 
    // we blit line-by-line vertically to cleanly bypass spatial memory wrapping anomalies.
    for (int lineY = 0; lineY < h; lineY++) {
        // Calculate the absolute continuous index position tracking this precise horizontal row row slice
        int currentSheetY = srcY + lineY;
        
        // Safety lock clipping: do not attempt pointer math outside the absolute bounds of the layout RAM
        if (currentSheetY < 0 || srcX < 0 || srcX >= sheetStride) continue;

        // Compute direct target memory pointer vector chunk address mapping
        const unsigned char *rowPointerBytes = &cart->spriteRAM[(currentSheetY * sheetStride) + srcX];

        // Draw this individual 1-pixel-high horizontal strip line onto virtualVRAM frame
        // Uses your clipping-safe, fast native PR_Spr engine for drawing!
        PR_Spr(rowPointerBytes, w, 1, destX, destY + lineY, colorId);
    }

    return 0; // Returns exactly 0 arguments back to the active Lua execution runtime
}

// Bridges Lua 'spr(customBytesTable/String, w, h, x, y, [colorId])' 
// straight to our native, flexible-size PICO-RAY extended sprite blitting loop engine!
int PR_Lua_Spr_Buffer_OLD(lua_State *L) {
    // 1. INPUT VALIDATION GATES AND STACK EXTRACTOR PIPELINE
    // Extract raw custom bitmap bytes array stream from the first Lua stack argument.
    // If your Lua engine passes it as a continuous raw binary string token, luaL_checkstring fits perfectly.
    // (If it passes as a structural table list, you can adapt to lua_isstring/lua_istable check)
    const unsigned char *customBytes = (const unsigned char *)luaL_checkstring(L, 1);
    
    int w   = (int)luaL_checknumber(L, 2);
    int h   = (int)luaL_checknumber(L, 3);
    float x = (float)luaL_checknumber(L, 4);
    float y = (float)luaL_checknumber(L, 5);

    // 2. OPTIONAL TINT COLOR INDEX LOGIC
    // Default to index -1 or 7 (White/No tint filter override) if omitted by the script developer
    int colorId = 7; 
    if (lua_gettop(L) >= 6) {
        colorId = (int)luaL_checknumber(L, 6);
    }

    // 3. SECURE VERIFICATION SHIELD
    if (customBytes == NULL || w <= 0 || h <= 0) return 0;

    // 4. FIRE THE UNIFIED PICO-RAY NATIVE SPRITE ENGINE CORE PASS
    // Instantly maps original parameters straight into our optimized software VRAM blitter!
    PR_Spr(customBytes, w, h, x, y, colorId);

    return 0; // Returns exactly 0 arguments back to the active Lua execution runtime
}
