#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include "../../framework.h"

// --- C-SIDE WRAPPER EXPOSED TO LUA ---

// Bridges Lua 'btn(id)' to scan inputs (0=Left, 1=Right, 2=Up, 3=Down)
// Returns a boolean back to the Lua runtime thread state matrix
int PR_Lua_Btn(lua_State *L) {
    int buttonId = (int)luaL_checknumber(L, 1);
    bool isDown = false;

    // Map standard PICO-8 layout controller indices to Raylib hardware keyboard constants
    switch (buttonId) {
        case 0: isDown = IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A); break; // Left
        case 1: isDown = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D); break; // Right
        case 2: isDown = IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W); break; // Up
        case 3: isDown = IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S); break; // Down
        default: isDown = false; break;
    }

    lua_pushboolean(L, isDown); // Push the result boolean truth verification flag to Lua stack
    return 1; // Returns exactly 1 argument back to Lua
}

// Bridges Lua 'btnp(id)' to scan edge-triggered keystrokes (0=Left, 1=Right, 2=Up, 3=Down)
// Returns true ONLY on the exact frame the key was pressed down, supporting keyboard repeat delays!
int PR_Lua_Btnp(lua_State *L) {
    int buttonId = (int)luaL_checknumber(L, 1);
    bool isPressed = false;

    // Leverage Raylib's native edge-trigger and repeat timing logic cleanly
    switch (buttonId) {
        case 0: isPressed = IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)  || IsKeyPressedRepeat(KEY_LEFT)  || IsKeyPressedRepeat(KEY_A); break;
        case 1: isPressed = IsKeyPressed(KEY_RIGHT) || IsKeyDown(KEY_D)     || IsKeyPressed(KEY_RIGHT)       || IsKeyPressedRepeat(KEY_D); break;
        case 2: isPressed = IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)  || IsKeyPressedRepeat(KEY_UP)    || IsKeyPressedRepeat(KEY_W); break;
        case 3: isPressed = IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)  || IsKeyPressedRepeat(KEY_DOWN)  || IsKeyPressedRepeat(KEY_S); break;
        default: isPressed = false; break;
    }

    lua_pushboolean(L, isPressed);
    return 1; // Returns exactly 1 boolean back to Lua stack
}

// Bridges Lua 'cart_info()' to expose metadata strings safely
int PR_Lua_CartInfo(lua_State *L) {
    CartridgeMeta *meta = PR_GetCartridgeMeta();
    
    if (meta != NULL && strlen(meta->name) > 0) {
        // We can push them as multiple string arguments back to Lua!
        lua_pushstring(L, meta->name);
        lua_pushstring(L, meta->author);
        lua_pushstring(L, meta->version);
        return 3; // Returns exactly 3 strings to the Lua stack row
    }
    
    // Fallback if it was a raw naked script without any meta headers
    lua_pushstring(L, "Naked Script");
    lua_pushstring(L, "Unknown");
    lua_pushstring(L, "0.1.0");
    return 3;
}

// Bridges Lua 'cls(color)' directly to native C 'PR_Cls(color)'
int PR_Lua_Cls(lua_State *L) {
    int colorId = (int)luaL_checknumber(L, 1); // Extract 1st integer param from Lua stack
    PR_Cls(colorId);
    return 0; // Returns 0 arguments to Lua
}

// Bridges Lua 'exit_os()' directly to native C 'PR_Callback_CloseApp()'
int PR_Lua_ExitOS(lua_State *L) {
    PR_Callback_CloseApp();
    return 0; // Returns 0 arguments to Lua
}

// Bridges Lua 'music_play("track.mp3")' natively to the kernel streamer
int PR_Lua_MusicPlay(lua_State *L) {
    const char *fileName = luaL_checkstring(L, 1);
    if (fileName != NULL) {
        PR_PlayMusic(fileName);
    }
    return 0;
}

// Bridges Lua 'music_stop()' natively to the kernel streamer
int PR_Lua_MusicStop(lua_State *L) {
    PR_StopMusic();
    return 0;
}

// Bridges Lua 'palt([col], [t])' utilizing a flat intermediate lookup mask array
int PR_Lua_PAlt(lua_State *L) {
    int argsCount = lua_gettop(L);
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL) return 0;

    // 1. If called without arguments, reset entirely to default layout state
    if (argsCount == 0) {
        for (int i = 0; i < 32; i++) {
            gfx->isColorTransparent[i] = (i == 0);
        }
        return 0;
    }

    // 2. Extract and validate parameters safely inside 32-color limits
    int colorId = (int)luaL_checknumber(L, 1);
    if (colorId < 0 || colorId > 31) return 0;

    // Default to true if transparency flag 't' is omitted by the user
    bool isTransparent = true;
    if (argsCount >= 2) {
        isTransparent = lua_toboolean(L, 2);
    }

    // 3. DIRECT INTERMEDIATE ARRAY INJECTION (No complex bitwise operations needed!)
    gfx->isColorTransparent[colorId] = isTransparent;

    return 0;
}

// Bridges Lua 'pset(x, y, color)' directly to native C 'PR_PSet(x, y, color)'
int PR_Lua_PSet(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    int colorId = (int)luaL_checknumber(L, 3);
    PR_PSet(x, y, colorId);
    return 0;
}

// Bridges Lua 'pget(x, y)' directly to native C 'PR_PGet(x, y)'
int PR_Lua_PGet(lua_State *L) {
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    lua_pushnumber(L, PR_PGet(x, y));
    return 1;
}

// Bridges Lua 'print(text, x, y, color)' directly to native C 'PR_Print(text, x, y, color)'
int PR_Lua_Print(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    int colorId = (int)luaL_checknumber(L, 4);
    PR_Print(text, x, y, colorId);
    return 0;
}

// Bridges the Lua script call 'printpro(2, "Hello", 10, 20, 7)' straight to the C-kernel!
int PR_Lua_PrintPro(lua_State *L) {
    // 1. Parse and extract all 5 parameters strictly from the incoming Lua stack
    int fontId        = (int)luaL_checkinteger(L, 1);
    const char *text  = luaL_checkstring(L, 2);
    float startX      = (float)luaL_checknumber(L, 3);
    float startY      = (float)luaL_checknumber(L, 4);
    int colorId       = (int)luaL_checkinteger(L, 5);

    // 2. Fire our newly engineered, multi-table baseline-aligned parametric printer
    PR_PrintPro(fontId, text, startX, startY, colorId);

    // Return 0 because this function pushes zero output parameters back onto the Lua stack
    return 0; 
}

// Bridges Lua 'rect(x, y, w, h, [color])' straight to our optimized C primitives engine
int PR_Lua_Rect(lua_State *L) {
    // 1. Extract screen geometry boundary coordinates safely from the Lua stack
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int w = (int)luaL_checknumber(L, 3);
    int h = (int)luaL_checknumber(L, 4);
    
    // 2. Color parameter logic: default to palette index 6 (Light Grey/White) if omitted by user
    int colorId = 6;
    if (lua_gettop(L) >= 5) {
        colorId = (int)luaL_checknumber(L, 5);
    }

    // 3. Fire our unified high-performance C software rasterizer lines block
    PR_Rect(x, y, w, h, colorId);
    
    return 0; // Returns exactly zero parameters back to the Lua runtime stream
}

// Bridges Lua 'rectfill(x, y, w, h, [color])' straight to our optimized C primitives engine
int PR_Lua_RectFill(lua_State *L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float w = (float)luaL_checknumber(L, 3);
    float h = (float)luaL_checknumber(L, 4);
    
    int colorId = 6;
    if (lua_gettop(L) >= 5) {
        colorId = (int)luaL_checknumber(L, 5);
    }

    // Fire our unified high-performance C software row-flood writer pass
    PR_RectFill(x, y, w, h, colorId);
    
    return 0;
}

// Bridges Lua 'set_font(1)' natively straight to the runtime core switcher
int PR_Lua_SetFont(lua_State *L) {
    int fontId = (int)luaL_checkinteger(L, 1);
    PR_SetActiveFontId(fontId);
    return 0;
}

// Bridges Lua 'sget(spriteId, x, y)'
int PR_Lua_SGet(lua_State *L) {
    // Pop exactly 2 positional arguments from the Lua virtual stack
    int sheetX = (int)luaL_checkinteger(L, 1);
    int sheetY = (int)luaL_checkinteger(L, 2);

    // Call our newly wired native hardware pointer function
    int colorId = PR_SGet(sheetX, sheetY);

    // Push the resulting color integer index back onto the stack for Lua
    lua_pushinteger(L, colorId);
    return 1; 
}

// Bridges Lua 'ssett(spriteId, x, y, colorId)'
int PR_Lua_SSet(lua_State *L) {
    int sheetX = (int)luaL_checkinteger(L, 1);
    int sheetY = (int)luaL_checkinteger(L, 2);
    
    int colorId;
    // Check if the optional color argument 'c' was provided in Lua slot 3
    if (lua_gettop(L) >= 3) {
        colorId = (int)luaL_checkinteger(L, 3);
    } else {
        // TODO:
        // Fallback: Fetch the system's active drawing color index from the graphics system!
        // GraphicsSystem *gfx = PR_GetGraphicsSystem();
        // colorId = (gfx != NULL) ? gfx->currentColor : 0; 
        colorId = 0;
    }

    PR_SSet(sheetX, sheetY, colorId);
    return 0; // Void return
}

// Bridges Lua 'sfx_play("coin.ogg")' natively to the one-shot mixer
int PR_Lua_SFXPlay(lua_State *L) {
    const char *fileName = luaL_checkstring(L, 1);
    if (fileName != NULL) {
        PR_PlaySFX(fileName);
    }
    return 0;
}
