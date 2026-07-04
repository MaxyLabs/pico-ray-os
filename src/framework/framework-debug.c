// --- src/framework/modules/framework-debug.c ---
#include <stdio.h>
#include <string.h>
#include "framework.h"

// Dynamic string builder buffer mapping active hardware variables values into readable rows
static char debugTextBuffer[2048] = {0};

void PR_ToggleDebugSystem(void) {
    KernelState *kernel = PR_GetKernelState();
    if (kernel == NULL) return;

    kernel->isDebugOpen = !kernel->isDebugOpen;
    if (kernel->isDebugOpen) {
        kernel->debugStage = DEBUG_STAGE_LIST;
        kernel->debugSelectedIndex = 0;
        kernel->debugScrollLineOffset = 0;
        memset(debugTextBuffer, 0, sizeof(debugTextBuffer));
    }
}

// Gathers telemetry from kernel matrices and serializes it straight into our display buffer
static void CompileHardwareTelemetry(DebugTarget target) {
    memset(debugTextBuffer, 0, sizeof(debugTextBuffer));
    KernelState *kernel = PR_GetKernelState();
    if (kernel == NULL) return;
    
    switch(target) {
        case DEBUG_TARGET_KERNEL:
            // FIX 1: Removed .isRunning field to match your exact configuration layout
            {
            const char *warnings[6] = {0};
            int warningCount = PR_GetKernelWarnings(warnings, 6);
            char warningBlock[512] = {0};

            if (warningCount <= 0) {
                snprintf(warningBlock, sizeof(warningBlock), "none");
            } else {
                for (int i = 0; i < warningCount; i++) {
                    char line[96];
                    snprintf(line, sizeof(line), "[%d] %s", i + 1, warnings[i]);
                    if ((int)strlen(warningBlock) > 0) {
                        strncat(warningBlock, "\\n", sizeof(warningBlock) - strlen(warningBlock) - 1);
                    }
                    strncat(warningBlock, line, sizeof(warningBlock) - strlen(warningBlock) - 1);
                }
            }

            snprintf(debugTextBuffer, sizeof(debugTextBuffer),
                "--- KERNEL STATE DUMP ---\n"
                "isAboutOpen: %s\n"
                "isPauseOpen: %s\n"
                "pauseIndex: %d\n"
                "sysVolume: %d%%\n"
                "debugStage: %d\n"
                "debugIndex: %d\n"
                "warnings:\n%s",
                kernel->isAboutOpen ? "TRUE" : "FALSE",
                kernel->isPauseMenuOpen ? "TRUE" : "FALSE",
                kernel->pauseSelectedIndex,
                kernel->systemVolume,
                kernel->debugStage,
                kernel->debugSelectedIndex,
                warningBlock);
            }
            break;
            
        case DEBUG_TARGET_MOUSE: {
            MouseState m = PR_GetMousePosition();
            snprintf(debugTextBuffer, sizeof(debugTextBuffer),
                "--- MOUSE SYSTEM DUMP ---\n"
                "Cursor X: %d px\n"
                "Cursor Y: %d px\n"
                "Left Down: %s\n"
                "Right Down: %s\n"
                "VRAM Collision: %s",
                m.x, m.y,
                IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? "TRUE" : "FALSE",
                IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ? "TRUE" : "FALSE",
                (m.x >= 0 && m.x < 128 && m.y >= 0 && m.y < 128) ? "INSIDE" : "OUTSIDE");
            break;
        }
            
        case DEBUG_TARGET_META: {
            CartridgeMeta *meta = PR_GetCartridgeMeta();
            if (!meta) { strcpy(debugTextBuffer, "ERROR: No Cartridge Meta Available!"); break; }
            snprintf(debugTextBuffer, sizeof(debugTextBuffer),
                "--- CARTRIDGE META DUMP ---\n"
                "Title: %s\n"
                "Author: %s\n"
                "Version: %s\n"
                "License: %s\n"
                "Mode Profile: %s",
                meta->name, meta->author, meta->version, meta->license, meta->mode);
            break;
        }
            
        case DEBUG_TARGET_CART_RAM: {
            CartridgeRAM *cart = PR_GetCartridgeRAM();
            if (!cart) { strcpy(debugTextBuffer, "ERROR: No Cartridge RAM Available!"); break; }
            snprintf(debugTextBuffer, sizeof(debugTextBuffer),
                "--- CARTRIDGE RAM TRANSPARENCY DUMP ---\n"
                "spriteRAM Ptr: %p\n"
                "mapRAM Ptr: %p\n"
                "luaRAM Ptr: %p\n"
                "sheetWidth: %d px\n"
                "sheetHeight: %d px\n"
                "spriteSize: %d bytes\n"
                "mapRAMSize: %d bytes\n"
                "--> spriteRAMIndex: %d <--\n"
                "--> mapRAMIndex:    %d <\n"
                "luaRAMIndex:    %d bytes\n"
                "luaRAMCapacity: %d bytes",
                (void*)cart->spriteRAM, (void*)cart->mapRAM, (void*)cart->luaRAM,
                cart->spriteAtlasColumns, cart->spriteAtlasRows, cart->spriteRAMSize, cart->mapRAMSize,
                cart->spriteRAMIndex, cart->mapRAMIndex, cart->luaRAMIndex, cart->luaRAMSize);
            break;
        }
            
        case DEBUG_TARGET_GRAPHICS: {
            GraphicsSystem *gfx = PR_GetGraphicsSystem();
            if (!gfx) { strcpy(debugTextBuffer, "ERROR: No Graphics Engine Available!"); break; }
            
            // FIX 2: Removed non-existent mask field, cleanly printing the array cache states
            snprintf(debugTextBuffer, sizeof(debugTextBuffer),
                "--- GRAPHICS SYSTEM DUMP ---\n"
                "virtualVRAM Ptr: %p\n"
                "vramWidth:  %d px\n"
                "vramHeight: %d px\n"
                "isColor0Trans: %s\n"
                "isColor1Trans: %s\n"
                "isColor4Trans: %s\n"
                "isColor7Trans: %s",
                (void*)gfx->virtualVRAM.data, gfx->virtualWidth, gfx->virtualHeight,
                gfx->isColorTransparent[0] ? "TRUE (Transparent)" : "FALSE",
                gfx->isColorTransparent[1] ? "TRUE (Transparent)" : "FALSE",
                gfx->isColorTransparent[4] ? "TRUE (Transparent)" : "FALSE",
                gfx->isColorTransparent[7] ? "TRUE (Transparent)" : "FALSE");
            break;
        }
        case DEBUG_TARGET_EDITOR: {
            // Fetch raw state layout from our freshly exposed editor pipeline hook
            // We cast it dynamically to access live operational sub-registers
            typedef struct {
                int selectedSpriteX;
                int selectedSpriteY;
                int activeColor;
                int hoveredPixelX;
                int hoveredPixelY;
                bool isGridActive;
                unsigned char copyBuffer[8][8];
            } LocalEditorState; // Mirror structure to parse bytes cleanly
            
            LocalEditorState *ed = (LocalEditorState*)PR_GetSpriteEditorState();
            CartridgeRAM *cart = PR_GetCartridgeRAM();
            
            if (!ed || !cart) { strcpy(debugTextBuffer, "ERROR: Editor Diagnostics Offline!"); break; }
            
            snprintf(debugTextBuffer, sizeof(debugTextBuffer),
                "--- SPRITE EDITOR TRACE ---\n"
                "Active Sprite:  ID %03d\n"
                "Grid Position:  X:%d Y:%d\n"
                "Brush Color ID: %d\n"
                "Grid Overlay:   %s\n"
                "Hovered Pixel:  X:%d Y:%d\n"
                "---------------------------\n"
                "LIVE VRAM SANITY CHECK:\n"
                "RAM Pointer:    %p\n"
                "Sheet Width:    %d px (Stride)\n"
                "RAM Parse Idx:  %d / 16384\n"
                "RAM Status:     %s",
                (ed->selectedSpriteY * 16) + ed->selectedSpriteX,
                ed->selectedSpriteX, ed->selectedSpriteY,
                ed->activeColor,
                ed->isGridActive ? "ACTIVE" : "DISABLED",
                ed->hoveredPixelX, ed->hoveredPixelY,
                (void*)cart->spriteRAM,
                cart->spriteAtlasColumns,
                cart->spriteRAMIndex,
                (cart->spriteRAMIndex == 16384) ? "LOADED & INTEGRAL" : "EMPTY OR TRUNCATED");
            break;
        }
        default: break;
    }
}

// Processes internal menu navigations inputs inside our debug console layer
void PR_UpdateDebugSystem(void) {
    KernelState *kernel = PR_GetKernelState();
    if (kernel == NULL || !kernel->isDebugOpen) return;

    if (kernel->debugStage == DEBUG_STAGE_LIST) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            kernel->debugSelectedIndex = (kernel->debugSelectedIndex - 1 + DEBUG_TARGET_COUNT) % DEBUG_TARGET_COUNT;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            kernel->debugSelectedIndex = (kernel->debugSelectedIndex + 1) % DEBUG_TARGET_COUNT;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            kernel->debugActiveTarget = (DebugTarget)kernel->debugSelectedIndex;
            CompileHardwareTelemetry(kernel->debugActiveTarget);
            kernel->debugStage = DEBUG_STAGE_DATA;
        }
    } 
    else if (kernel->debugStage == DEBUG_STAGE_DATA) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) { 
            if (kernel->debugScrollLineOffset > 0) kernel->debugScrollLineOffset--; 
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { 
            kernel->debugScrollLineOffset++; 
        }
        
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_B)) {
            kernel->debugStage = DEBUG_STAGE_LIST;
        }
        if (IsKeyPressed(KEY_P)) {
            // 1. STANDARD TELEMETRY SCREEN LOG PRINT
            printf("\n=== PICO-RAY HARDWARE TELEMETRY CONSOLE PRINT ===\n%s\n", debugTextBuffer);
                
            CartridgeRAM *cart = PR_GetCartridgeRAM();
            KernelState *kernel = PR_GetKernelState();
                
            if (cart != NULL && kernel != NULL) {
                
                // ====================================================================
                // NEW ADVANCED DIAGNOSTICS: RAW luaRAM HEXDUMP INSPECTOR
                // This will print the first 512 bytes of the raw lua memory block 
                // in a professional Hex + ASCII side-by-side terminal format!
                // ====================================================================
                if (cart->luaRAM != NULL) {
                    printf("\n>>> KERNEL CORE luaRAM RAW COMPILER DUMP (FIRST 512 BYTES HEXDUMP) <<<\n");
                    printf("Address Ptr: %p | Active Size Counter: %d bytes\n\n", (void*)cart->luaRAM, cart->luaRAMIndex);
                    printf("Offset    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F   ASCII Channel\n");
                    printf("-------------------------------------------------------------------------\n");
                
                    // Inspect the first 512 byte slots of the buffer area
                    for (int i = 0; i < 512; i++) {
                        // Print address row offset token at every 16 bytes stride boundary
                        if (i % 16 == 0) {
                            printf("0x%04X:   ", i);
                        }
                    
                        unsigned char byteValue = (unsigned char)cart->luaRAM[i];
                        printf("%02X ", byteValue);
                    
                        // Side-by-side ASCII mapping visualization column rendering at row end
                        if ((i + 1) % 16 == 0) {
                            printf("  | ");
                            for (int j = i - 15; j <= i; j++) {
                                unsigned char ch = (unsigned char)cart->luaRAM[j];
                                // If character is printable string text, print it, otherwise show a dot spacer
                                if (ch >= 32 && ch <= 126) {
                                    putchar(ch);
                                } else {
                                    putchar('.');
                                }
                            }
                            printf(" |\n");
                        }
                    }
                    printf(">>> END OF luaRAM RAW HEXDUMP <<<\n\n");
                }
            
                // ==========================================
                // EXISTING spriteRAM GRAPHICS DUMP
                // ==========================================
                if (cart->spriteRAM != NULL && 
                   (kernel->debugActiveTarget == DEBUG_TARGET_CART_RAM || kernel->debugActiveTarget == DEBUG_TARGET_EDITOR)) {
                    
                    printf("\n>>> KERNEL CORE spriteRAM MEMORY DUMP (128x128 MATRIX) <<<\n");
                    for (int y = 0; y < 128; y++) {
                        printf("/* Row %03d */ \"", y);
                        for (int x = 0; x < 128; x++) {
                            int memIdx = (y * 128) + x;
                            unsigned char colorIdx = cart->spriteRAM[memIdx] & 0x1F;
                            if (colorIdx == 0)       putchar('.');
                            else if (colorIdx < 10)  putchar('0' + colorIdx);
                            else if (colorIdx < 16)  putchar('A' + (colorIdx - 10));
                            else                     putchar('G' + (colorIdx - 16));
                        }
                        printf("\",\n");
                    }
                    printf(">>> END OF spriteRAM HARDWARE DUMP <<<\n\n");
                }
            }
            printf("==================================================\n");
        }
    }
}

// Draws the system introspection window overlay
void PR_DrawDebugSystem(void) {
    KernelState *kernel = PR_GetKernelState();
    if (kernel == NULL || !kernel->isDebugOpen) return;

    // 1. FULLSCREEN CLEAN BACKDROP OVERLAY
    // Wash the entire 128x128 matrix with pure solid black to maximize terminal readability
    PR_RectFill(0, 0, 128, 128, P8_BLACK);

    // 2. COMPACT SYSTEM HEADER BAR (Sits exactly at the top 9 pixels row)
    PR_RectFill(0, 0, 128, 9, P8_DARK_GREEN);
    PR_Print("PICO-RAY CORE INSPECTOR", 4, 2, P8_WHITE);
    PR_Line(0, 9, 128, 9, P8_DARK_GREY);

    // FOOTER CONFIGURATIONS: Common geometry layout constraints
    int footerY = 119;

    // ==========================================
    // STAGE A: FULLSCREEN INTERACTIVE TARGETS LIST
    // ==========================================
    if (kernel->debugStage == DEBUG_STAGE_LIST) {
        const char *listItems[] = {
            "1. KernelState",
            "2. MouseState",
            "3. CartridgeMeta",
            "4. CartridgeRAM",
            "5. GraphicsSystem",
            "6. SpriteEditorState"
        };

        PR_Print("SELECT MONITOR NODE:", 6, 14, P8_BLUE);

        // Render the selectable rows spanning nicely across the full width
        for (int i = 0; i < DEBUG_TARGET_COUNT; i++) {
            int rowY = 26 + (i * 11);
            if (i == kernel->debugSelectedIndex) {
                // Highlight line bar takes full screen horizontal span with a 2px margin
                PR_RectFill(2, rowY - 1, 124, 9, P8_BLUE);
                PR_Print(listItems[i], 6, rowY, P8_YELLOW);
            } else {
                PR_Print(listItems[i], 6, rowY, P8_LIGHT_GREY);
            }
        }
        
        // Fullscreen system prompt footer row bar
        PR_RectFill(0, footerY, 128, 9, P8_DARK_GREY);
        PR_Print("[ENTER] INJECT   [F4] EXIT", 4, footerY + 2, P8_LIGHT_GREY);
    }   
    // ==========================================
    // STAGE B: MAXIMIZED SCROLLABLE DATA TEXTBOX
    // ==========================================
    else if (kernel->debugStage == DEBUG_STAGE_DATA) {
        // FIX: Expanded text boundary box to occupy maximum possible spatial VRAM matrix area!
        // Sits perfectly between the 9px top header and the 9px bottom footer panel strips.
        int boxX = 2;
        int boxY = 12;
        int boxW = 124;
        int boxH = 104; // Drastically increased from 72px to 104px! Double the vertical workspace view!

        // Subtle thin layout indicator border for terminal encapsulation feeling
        PR_Rect(boxX, boxY, boxW, boxH, P8_DARK_GREY);

        char textCopy[2048];
        strcpy(textCopy, debugTextBuffer);
        
        char *lineToken = strtok(textCopy, "\n");
        int currentLineIdx = 0;
        int drawnLinesCount = 0;

        // Iterate rows plotting lines directly onto maximized canvas steps
        while (lineToken != NULL) {
            if (currentLineIdx >= kernel->debugScrollLineOffset) {
                int printY = boxY + 3 + (drawnLinesCount * 8);
                // Dynamically evaluate if the next text row fits inside the newly expanded box height boundary
                if (printY + 7 < boxY + boxH) {
                    PR_Print(lineToken, boxX + 4, printY, P8_GREEN); // Classic matrix green text tone!
                    drawnLinesCount++;
                }
            }
            currentLineIdx++;
            lineToken = strtok(NULL, "\n");
        }

        // Fullscreen telemetry data prompt footer row bar
        PR_RectFill(0, footerY, 128, 9, P8_DARK_GREY);
        PR_Print("[B] BACK     [P] PRINT TO PC", 4, footerY + 2, P8_YELLOW);
    }
}
