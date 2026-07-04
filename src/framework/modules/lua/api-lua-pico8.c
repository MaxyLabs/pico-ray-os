#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "../../framework.h"

// Bridges Lua 'mget(tileX, tileY)' to official PICO-8 map query standards
int P8_Lua_MGet(lua_State *L) {
    // Official PICO-8 map coordinates are strictly 0-indexed! 
    // We pass coordinates directly without any automatic '- 1' subtractions!
    int tx = (int)luaL_checknumber(L, 1); 
    int ty = (int)luaL_checknumber(L, 2);
    
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    int value = 0;
    
    if (cart != NULL && cart->mapRAM != NULL && tx >= 0 && tx < cart->mapColumns && ty >= 0 && ty < cart->mapRows) {
        int memoryCellOffset = (ty * cart->mapColumns) + tx;
        value = (int)cart->mapRAM[memoryCellOffset];
    }
    
    lua_pushnumber(L, value);
    return 1;
}

// Bridges Lua 'mset(tileX, tileY, value)' to official PICO-8 map mutation standards
int P8_Lua_MSet(lua_State *L) {
    int tx = (int)luaL_checknumber(L, 1);
    int ty = (int)luaL_checknumber(L, 2);
    unsigned char val = (unsigned char)luaL_checknumber(L, 3);

    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart != NULL && cart->mapRAM != NULL && tx >= 0 && tx < cart->mapColumns && ty >= 0 && ty < cart->mapRows) {
        int memoryCellOffset = (ty * cart->mapColumns) + tx;
        cart->mapRAM[memoryCellOffset] = val;
    }
    return 0;
}

// Bridges Lua PICO-8 'spr(n, [x, y], [w, h], [flip_x], [flip_y])' straight to C renderer
int P8_Lua_Spr(lua_State *L) {
    int argsCount = lua_gettop(L);
    if (argsCount < 1) return 0; // Guard protection

    // Extract mandatory sprite identifier index number
    int n = (int)luaL_checknumber(L, 1);

    // Default parameters matching the official PICO-8 specifications rules
    float destX = 0.0f;
    float destY = 0.0f;
    float w     = 1.0f;
    float h     = 1.0f;
    bool flipX  = false;
    bool flipY  = false;

    // Unpack arguments from the stack sequentially matching active counts
    if (argsCount >= 3) {
        destX = (float)luaL_checknumber(L, 2);
        destY = (float)luaL_checknumber(L, 3);
    }
    if (argsCount >= 5) {
        w = (float)luaL_checknumber(L, 4);
        h = (float)luaL_checknumber(L, 5);
    }
    if (argsCount >= 6) {
        flipX = lua_toboolean(L, 6);
    }
    if (argsCount >= 7) {
        flipY = lua_toboolean(L, 7);
    }

    // FIX: Clean, single-line delegation straight to our standalone C drawing core!
    P8_Spr(n, destX, destY, w, h, flipX, flipY);

    return 0; // Zero values pushed back to Lua runtime thread
}

