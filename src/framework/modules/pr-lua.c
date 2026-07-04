#include <ctype.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>
#include "../framework.h"

void PR_Lua_LoadAPI(lua_State *L) {
    // REGISTER UNIFIED BASELINE METHODS COMMON TO ALL MODES
    lua_register(L, "cls",        PR_Lua_Cls);       // CLear screen
    lua_register(L, "btn",        PR_Lua_Btn);       // Input register
    lua_register(L, "btnp",       PR_Lua_Btnp);
    lua_register(L, "exit_os",    PR_Lua_ExitOS);    // Register our fresh close command keyword
    lua_register(L, "pget",       PR_Lua_PGet);      // Get pixel color
    lua_register(L, "pset",       PR_Lua_PSet);      // Set pixel color
    lua_register(L, "print",      PR_Lua_Print);     // Print text
    lua_register(L, "printpro",   PR_Lua_PrintPro);  // Print text using a specific font
    lua_register(L, "rect",       PR_Lua_Rect);      // Draw hollow rectangle border
    lua_register(L, "rectfill",   PR_Lua_RectFill);  // Draw filled solid rectangle
    lua_register(L, "set_font",   PR_Lua_SetFont);
    lua_register(L, "sget",       PR_Lua_SGet);
    lua_register(L, "sset",       PR_Lua_SSet);
}

// Master registration gateway injecting our advanced extended blueprint rules
void PR_Lua_LoadPicoRayAPI(lua_State *L) {
    // REGISTER METHODS EXCLUSIVE TO PICORAY
    lua_register(L, "mget",  PR_Lua_MGet);   // Map query register
    lua_register(L, "mset",  PR_Lua_MSet);   // Map write register
    lua_register(L, "spr",   PR_Lua_Spr);    // Draw a sprite
}

// REGISTER METHODS EXCLUSIVE TO PCIO-8
void PR_Lua_LoadPico8CompatibilityAPI(lua_State *L) {
    lua_register(L, "mget",  P8_Lua_MGet);
    lua_register(L, "mset",  P8_Lua_MSet);
    lua_register(L, "spr",   P8_Lua_Spr);    // Draw a sprite (8x8)
}

void PR_Lua_RegisterAudioAPI(lua_State *L) {
    lua_register(L, "music_play", PR_Lua_MusicPlay);
    lua_register(L, "music_stop", PR_Lua_MusicStop);
    lua_register(L, "sfx_play",   PR_Lua_SFXPlay);
}

// Highly intelligent lexical pre-parser converting retro operators (+=, -=) to standard Lua syntax.
// Actively tracks comments and strings boundaries to guarantee zero data pollution!
char* PR_PreParseLuaSyntax(const char* sourceCode) {
    if (sourceCode == NULL) return NULL;

    size_t sourceLen = strlen(sourceCode);
    // Allocate a generous safety output buffer (up to 2x source size to comfortably fit string expansions)
    size_t destCapacity = (sourceLen * 2) + 256;
    char* dest = (char*)malloc(destCapacity);
    if (!dest) return NULL;
    memset(dest, 0, destCapacity);

    size_t sIdx = 0; // Source cursor index
    size_t dIdx = 0; // Destination cursor index

    // State machine configuration flags
    bool isInSingleQuoteString = false;
    bool isInDoubleQuoteString = false;
    bool isInLineComment = false;

    // FIX 3: Corrected explicit array type definition from single char to bounded buffer string row!
    char currentVarName[128];
    size_t varLen = 0;
    memset(currentVarName, 0, sizeof(currentVarName));

    while (sIdx < sourceLen) {
        char c = sourceCode[sIdx];
        char nextC = (sIdx + 1 < sourceLen) ? sourceCode[sIdx + 1] : '\0';

        // A. STATE TRACKER: LINE COMMENTS LAYER (-- ...)
        if (isInLineComment) {
            dest[dIdx++] = sourceCode[sIdx++];
            if (c == '\n') isInLineComment = false; // Reset state on newline boundary
            varLen = 0; // Clear variable cache tracker
            continue;
        }

        // B. STATE TRACKER: STRINGS ESCAPE LAYERS ('...' or "...")
        if (isInSingleQuoteString) {
            dest[dIdx++] = sourceCode[sIdx++];
            // FIX 4: Secure pointer subtraction history tracking to check backslash escapes safely
            if (c == '\'' && sIdx >= 2 && sourceCode[sIdx - 2] != '\\') isInSingleQuoteString = false;
            varLen = 0;
            continue;
        }
        if (isInDoubleQuoteString) {
            dest[dIdx++] = sourceCode[sIdx++];
            if (c == '"' && sIdx >= 2 && sourceCode[sIdx - 2] != '\\') isInDoubleQuoteString = false;
            varLen = 0;
            continue;
        }

        // C. CORE TRIGGER GATES ENTERING BOUNDARY STATES
        if (c == '-' && nextC == '-') {
            isInLineComment = true;
            dest[dIdx++] = sourceCode[sIdx++];
            dest[dIdx++] = sourceCode[sIdx++];
            varLen = 0;
            continue;
        }
        if (c == '\'') {
            isInSingleQuoteString = true;
            dest[dIdx++] = sourceCode[sIdx++];
            varLen = 0;
            continue;
        }
        if (c == '"') {
            isInDoubleQuoteString = true;
            dest[dIdx++] = sourceCode[sIdx++];
            varLen = 0;
            continue;
        }

        // D. IDENTIFIER TRACKING ENGINE 
        if (isalnum((unsigned char)c) || c == '_') {
            if (varLen < 127) {
                currentVarName[varLen++] = c;
                currentVarName[varLen] = '\0';
            }
        } 
        // Ignore whitespaces while compiling token tracks to keep variable scopes aligned
        else if (c != ' ' && c != '\t') {
            // If any other symbol breaks the flow (except the actual target operator), clear tracker
            if (!(c == '+' && nextC == '=') && !(c == '-' && nextC == '=')) {
                varLen = 0;
                currentVarName[0] = '\0';
            }
        }

        // E. THE REVOLUTIONARY OPERATOR RE-WRITER CHANNELS (+= and -=)
        if (varLen > 0 && ((c == '+' && nextC == '=') || (c == '-' && nextC == '='))) {
            char opSymbol = c; // Remember if it was '+' or '-'

            // Strip trailing whitespaces behind the destination cursor to re-align the equals sign cleanly
            while (dIdx > 0 && (dest[dIdx - 1] == ' ' || dest[dIdx - 1] == '\t')) {
                dIdx--;
            }

            // Inject the standard assignment equals sign token sequence -> " = "
            dest[dIdx++] = ' ';
            dest[dIdx++] = '=';
            dest[dIdx++] = ' ';

            // Inject the cached variable name token sequence -> e.g. "player_x"
            for (size_t v = 0; v < varLen; v++) {
                dest[dIdx++] = currentVarName[v];
            }

            // Inject the expanded operational math sign -> e.g. " + " or " - "
            dest[dIdx++] = ' ';
            dest[dIdx++] = opSymbol;
            dest[dIdx++] = ' ';

            sIdx += 2; // Advance source cursor past both characters of the macro operator (+= / -=)
            
            // Clear current tracker matrices for the next lines
            varLen = 0;
            currentVarName[0] = '\0';
            continue;
        }

        // Standard copy mechanism for baseline normal tokens
        dest[dIdx++] = sourceCode[sIdx++];
    }

    dest[dIdx] = '\0'; // Seal destination string cleanly
    return dest;
}
