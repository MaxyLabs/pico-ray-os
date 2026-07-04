#include <dlfcn.h> // EZ KELL: A natív macOS .dylib betöltő könyvtár!
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../framework.h"

extern AppInterface Plugin_App;

#define LUA_RAM_DEFAULT_SIZE (64 * 1024)

static bool EnsureSpriteRAMCapacity(CartridgeRAM *cart, int requiredCapacity) {
    if (cart == NULL || requiredCapacity <= 0) return false;
    if (cart->spriteRAMSize >= requiredCapacity) return true;

    int newCapacity = cart->spriteRAMSize;
    if (newCapacity <= 0) newCapacity = requiredCapacity;
    while (newCapacity < requiredCapacity) {
        newCapacity *= 2;
    }

    unsigned char *resized = (unsigned char *)realloc(cart->spriteRAM, (size_t)newCapacity);
    if (resized == NULL) {
        return false;
    }

    // Clear newly allocated region so untouched bytes remain deterministic.
    memset(resized + cart->spriteRAMSize, 0, (size_t)(newCapacity - cart->spriteRAMSize));
    cart->spriteRAM = resized;
    cart->spriteRAMSize = newCapacity;
    return true;
}

// Cartridge
static void* loadedCartHandle = NULL; // A betöltött dylib mutatója

void PR_LoadPluginCartridge(void) {
    // --- HARMADIK FÉL JÁTÉKÁNAK BETÖLTÉSE ---
    if (loadedCartHandle != NULL) {
        dlclose(loadedCartHandle); // Korábbi dylib lezárása
        loadedCartHandle = NULL;
    }

    // macOS natív dylib megnyitás (RTLD_LAZY = azonnali betöltés futásidőben)
    loadedCartHandle = dlopen("carts/custom_game.dylib", RTLD_LAZY);

    if (loadedCartHandle != NULL) {
        // Függvények megkeresése a binárisban
        AppConfig (*extGetConfig)(void) = (AppConfig(*)(void))dlsym(loadedCartHandle, "Plugin_GetConfig");
        void (*extInit)(void)      = dlsym(loadedCartHandle, "Plugin_Init");
        void (*extUpdate)(Vector2) = dlsym(loadedCartHandle, "Plugin_Update");
        void (*extDraw)(void)      = dlsym(loadedCartHandle, "Plugin_Draw");
        void (*extCleanup)()       = dlsym(loadedCartHandle, "Plugin_Cleanup");

        if (extInit != NULL && extUpdate != NULL && extDraw != NULL) {
            // Sikeres betöltés! Átadjuk a 4 függvénymutatót a Frameworknek
            SwitchApp(Plugin_App); 
        } else {
            printf("ERROR: Required funtions are missing in dylib!\n");
        }
    } else {
        printf("ERROR: Failed to load dylib: %s\n", dlerror());
    }
}

bool PR_LoadRayCartridge(const char *filePath, CartridgeRAM *cart) {
    if (filePath == NULL || cart == NULL) return false;
    if (cart->spriteRAM == NULL || cart->mapRAM == NULL) return false;

    const CartridgeRAM *defaultCart = PR_GetDefaultCartridgeRAM();

    // We MUST force the tracking index registers to zero BEFORE checking buffers or allocations!
    // This wipes any leftover size values from previous game sessions and forces cursors to the start.
    cart->spriteRAMIndex = 0;
    cart->mapRAMIndex = 0;
    cart->luaRAMIndex = 0; // Safe zero start anchor!

    // Ensure Lua text storage exists before parsing __lua__.
    if (cart->luaRAM == NULL || cart->luaRAMSize <= 0) {
        cart->luaRAMSize = LUA_RAM_DEFAULT_SIZE;
        cart->luaRAM = (char *)calloc((size_t)cart->luaRAMSize, sizeof(char));
        if (cart->luaRAM == NULL) {
            return false;
        }
    }

    // HARDWARE INDEX REGISTERS INITIALIZATION
    cart->spriteAtlasColumns = defaultCart->spriteAtlasColumns;
    cart->spriteAtlasRows = defaultCart->spriteAtlasRows;
    cart->mapColumns = defaultCart->mapColumns;
    cart->mapRows = defaultCart->mapRows;

    // Securely wipe the fresh or existing buffer with clean zeros
    memset(cart->luaRAM, 0, (size_t)cart->luaRAMSize);

    CartridgeMeta *meta = PR_GetCartridgeMeta();
    if (meta != NULL) {
        memset(meta, 0, sizeof(*meta));
        strncpy(meta->mode, "picoray", sizeof(meta->mode) - 1);
    }

    FILE *file = fopen(filePath, "r");
    if (!file) return false;

    char line[512]; // Ensure buffer line is safely allocated
    int currentSection = SECTION_NONE;
    bool spriteStrideLocked = false;

    while (fgets(line, sizeof(line), file)) {
        // Instead of strcspn (which can aggressively truncate valid trailing pixels 
        // if file-end alignments or buffers shift), we cleanly trim backwards from the absolute end!
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        // SECTION SWITCH DETECTORS DIAGNOSTICS
        if (strcmp(line, "__meta__") == 0) { currentSection = SECTION_META; continue; }
        if (strcmp(line, "__gfx__")  == 0) { currentSection = SECTION_GFX;  continue; }
        if (strcmp(line, "__map__")  == 0) { currentSection = SECTION_MAP;  continue; }
        if (strcmp(line, "__lua__")  == 0) { currentSection = SECTION_LUA;  continue; }

        // ==========================================
        // __meta__ DATA PROCESSING BLOCK!
        // This extracts key=value pairs and populates the system metadata register.
        // Without this, the kernel remains blind to the requested 'mode=pico8' flag!
        // ==========================================
        if (currentSection == SECTION_META && meta != NULL) {
            char *equals = strchr(line, '=');
            if (equals != NULL) {
                *equals = '\0';
                const char *key = line;
                const char *val = equals + 1;

                if (strcmp(key, "name") == 0) {
                    strncpy(meta->name, val, sizeof(meta->name) - 1);
                } else if (strcmp(key, "author") == 0) {
                    strncpy(meta->author, val, sizeof(meta->author) - 1);
                } else if (strcmp(key, "version") == 0) {
                    strncpy(meta->version, val, sizeof(meta->version) - 1);
                } else if (strcmp(key, "license") == 0) {
                    strncpy(meta->license, val, sizeof(meta->license) - 1);
                } else if (strcmp(key, "mode") == 0) {
                    strncpy(meta->mode, val, sizeof(meta->mode) - 1);
                }
            }
        }
        // ==========================================
        // 2. UNIFIED __gfx__ DYNAMIC FLEXIBLE STRIDE PARSER LAYER WITH AUTOPADDING AND TRUNCATION
        // ==========================================
        if (currentSection == SECTION_GFX && cart != NULL && cart->spriteRAM != NULL) {
            int lineLen = (int)strlen(line);
            int validPixelsInThisLine = 0;

            // Count useful non-whitespace data tokens in this raw line buffer
            int rawDataWidth = 0;
            for (int w = 0; w < lineLen; w++) {
                if (line[w] != ' ' && line[w] != '\t' && line[w] != '\r' && line[w] != '\n') {
                    rawDataWidth++;
                }
            }

            // Lock sprite sheet stride from the first non-empty __gfx__ line.
            if (!spriteStrideLocked && rawDataWidth > 0) {
                cart->spriteAtlasColumns = rawDataWidth;
                spriteStrideLocked = true;
                printf("PICO-RAY OS | PARSER: Locked dynamic gfx asset sheet stride width to %d px.\n", cart->spriteAtlasColumns);
            }
        
            // Ensure we have a valid stride anchor to prevent division by zero anomalies
            int targetStride = (cart->spriteAtlasColumns > 0) ? cart->spriteAtlasColumns : defaultCart->spriteAtlasColumns;

            if (validPixelsInThisLine > 0 || rawDataWidth > 0) {
                int required = cart->spriteRAMIndex + targetStride;
                if (!EnsureSpriteRAMCapacity(cart, required)) {
                    fclose(file);
                    return false;
                }
            }
        
            // PROCESS INDIVIDUAL PIXELS WITH STRICT BOUNDARY TRUNCATION OVERRIDE
            for (int i = 0; i < lineLen; i++) {
                char rawCh = line[i];
                if (rawCh == ' ' || rawCh == '\t' || rawCh == '\r' || rawCh == '\n') continue;
            
                // FIX 1: FORM-ERROR TRUNCATION SHIELD
                // If this row contains MORE pixels than the locked baseline layout width marker,
                // kíméletlenül eldobjuk a többletet (skip processing), treating it as a format anomaly!
                if (validPixelsInThisLine >= targetStride) {
                    continue;
                }
            
                char ch = toupper((unsigned char)rawCh);
                unsigned char colorValue = 0;
            
                if (ch == '.')           colorValue = 0;
                else if (ch >= '0' && ch <= '9') colorValue = ch - '0';
                else if (ch >= 'A' && ch <= 'V') {
                    colorValue = (ch <= 'F') ? (10 + (ch - 'A')) : (16 + (ch - 'G'));
                } else {
                    colorValue = 0; 
                }
            
                // Write directly into our continuous 1D hardware array up to allocated limits
                cart->spriteRAM[cart->spriteRAMIndex++] = colorValue & 0x1F;
                validPixelsInThisLine++;
            }
        
            // FIX 2: AUTOMATIC HARDWARE ROW AUTOPADDING
            // If the read row line was shorter than our locked baseline layout stride,
            // we step in and fill the remaining gap slice with clean 0 (transparent) padding bytes!
            // This forces the next line to always start precisely at the next stride boundary index.
            if (validPixelsInThisLine > 0 && validPixelsInThisLine < targetStride) {
                int paddingNeeded = targetStride - validPixelsInThisLine;

                for (int p = 0; p < paddingNeeded; p++) {
                    cart->spriteRAM[cart->spriteRAMIndex++] = 0; // Pad out row safely
                }
            }
        }

        // ==========================================
        // 3. UNIFIED __map__ TILEMAP PARSER LAYER WITH DYNAMIC STRIDE & ROW PADDING
        // ==========================================
        if (currentSection == SECTION_MAP && cart != NULL && cart->mapRAM != NULL) {
            int lineLen = (int)strlen(line);
            int validTilesInThisLine = 0;
            
            // Analyze the active line buffer, counting valid non-whitespace tile tokens
            int rawDataWidth = 0;
            for (int w = 0; w < lineLen; w++) {
                if (line[w] != ' ' && line[w] != '\t' && line[w] != '\r' && line[w] != '\n') {
                    rawDataWidth++;
                }
            }

            // A. HARDWARE PROFILE ASSIGNMENT SWITCHER
            if (meta != NULL && strcmp(meta->mode, "pico8") == 0) {
                // PICO-8 execution profiles strictly enforce a default map sheet width stride of 128 tiles!
                cart->mapColumns = defaultCart->mapColumns;
                cart->mapRows = defaultCart->mapRows;
            } 
            else if (cart->mapColumns == 0 && rawDataWidth > 0) {
                // PICO-RAY Extended Fallback: The very first read map row dynamically locks the custom layout stride!
                cart->mapColumns = rawDataWidth;
                printf("PICO-RAY OS | PARSER: Locked dynamic tilemap stride width to %d tiles.\n", cart->mapColumns);
            }

            // Safeguard active target stride calculation to prevent division by zero anomalies
            int targetMapStride = (cart->mapColumns > 0) ? cart->mapColumns : 32;

            // B. PROCESS INDIVIDUAL TILE PIXELS WITH BOUNDARY TRUNCATION SHIELD
            for (int i = 0; i < lineLen; i++) {
                char rawCh = line[i];
                if (rawCh == ' ' || rawCh == '\t' || rawCh == '\r' || rawCh == '\n') continue;

                // FORM-ERROR TRUNCATION SHIELD
                // If a manual edit line exceeds the locked stride width anchor, discard the overflowing trailing data cleanly!
                if (validTilesInThisLine >= targetMapStride) {
                    continue; 
                }

                char ch = toupper((unsigned char)rawCh);
                unsigned char tileValue = 0;

                if (ch == '.') {                 tileValue = 0; }
                else if (ch >= '0' && ch <= '9') tileValue = ch - '0';
                else if (ch >= 'A' && ch <= 'V') {
                    tileValue = (ch <= 'F') ? (10 + (ch - 'A')) : (16 + (ch - 'G'));
                } else {
                    tileValue = 0; // Unrecognized format characters collapse to empty air/floor index 0 safely
                }

                // Inject directly inside the linear 1D mapRAM array up to allocated bounds limit
                if (cart->mapRAMIndex < cart->mapRAMSize) {
                    cart->mapRAM[cart->mapRAMIndex++] = tileValue & 0x1F; // Clamped strictly within 32 tile ids
                    validTilesInThisLine++;
                }
            }

            // C. AUTOMATIC HARDWARE TILEMAP ROW AUTOPADDING
            // If the read file row was shorter than our locked target stride layout, 
            // flood the remaining tail gaps with clean index 0 tiles to keep row structures aligned!
            if (validTilesInThisLine > 0 && validTilesInThisLine < targetMapStride) {
                int paddingNeeded = targetMapStride - validTilesInThisLine;
                
                for (int p = 0; p < paddingNeeded; p++) {
                    if (cart->mapRAMIndex < cart->mapRAMSize) {
                        cart->mapRAM[cart->mapRAMIndex++] = 0; // Safe row alignment padding
                    }
                }
            }
        }
        // ==========================================
        // 4. UNIFIED __lua__ SCRIPT PARSER LAYER
        // ==========================================
        if (currentSection == SECTION_LUA) {
            size_t lineLen = strlen(line);
            size_t available = (size_t)cart->luaRAMSize - (size_t)cart->luaRAMIndex;

            if (available > 1) {
                size_t copyLen = lineLen;
                if (copyLen + 2 > available) {
                    copyLen = available - 2;
                }

                if (copyLen > 0) {
                    memcpy(cart->luaRAM + cart->luaRAMIndex, line, copyLen);
                    cart->luaRAMIndex += (int)copyLen;
                }

                cart->luaRAM[cart->luaRAMIndex++] = '\n';
                cart->luaRAM[cart->luaRAMIndex] = '\0';
            }
        }
    }

    fclose(file);

    if (cart->spriteAtlasColumns > 0) {
        cart->spriteAtlasRows = cart->spriteRAMIndex / cart->spriteAtlasColumns;
    }
    
    // FIX 3: SAFE POST-PARSING CLEANER AUTOMATISM
    // Wipe the remaining unused trailing layout bytes *only* if the files were shorter than maximum capacity
    if (cart->spriteRAMIndex > 0 && cart->spriteRAMIndex < cart->spriteRAMSize) {
        memset(&cart->spriteRAM[cart->spriteRAMIndex], 0, (size_t)(cart->spriteRAMSize - cart->spriteRAMIndex));
    }
    if (cart->mapRAMIndex > 0 && cart->mapRAMIndex < cart->mapRAMSize) {
        memset(&cart->mapRAM[cart->mapRAMIndex], 0, (size_t)(cart->mapRAMSize - cart->mapRAMIndex));
    }

    return true;
}
