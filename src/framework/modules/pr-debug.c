#include <stdio.h>
#include "../framework.h"

// --- CORE SYSTEM VRAM BUFFER DUMPER ---
// Scans the active 128x128 screen pixel-by-pixel using PR_PGet and dumps a pure hex matrix string to stdout terminal
// --- CORE SYSTEM VRAM BUFFER DUMPER ---
void PR_Debug_DumpVRAM(void) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL) return;

    int targetW = gfx->virtualWidth;
    int targetH = gfx->virtualHeight;

    printf("\n--- PICO-RAY OS | ACTIVE SOFTWARE VRAM MEMORY DUMP START ---\n");
    
    for (int y = 0; y < targetH; y++) {
        printf("/* Row %03d */ \"", y);
        
        for (int x = 0; x < targetW; x++) {
            int rawColor = PR_PGet(x, y);
            
            // FIX: Enforce a strict 5-bit mask utility to extract ONLY the clean 0-31 palette index channels,
            // bypassing any raw RGBA hex background values completely!
            int colorIndex = rawColor & 0x1F; 

            // Convert explicitly to a single secure ASCII character
            char hexChar;
            if (colorIndex < 10) {
                hexChar = '0' + colorIndex;
            } else {
                hexChar = 'A' + (colorIndex - 10);
            }
            
            // Output exactly ONE single character block, keeping row string width perfectly flat!
            printf("%c", hexChar);
        }
        printf("\",\n");
    }
    printf("--- PICO-RAY OS | ACTIVE SOFTWARE VRAM MEMORY DUMP END ---\n\n");
}

void PR_Debug_DumpVRAM_old(void) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL) return;

    printf("\n--- PICO-RAY OS | ACTIVE SOFTWARE VRAM MEMORY DUMP START ---\n");
    
    for (int y = 0; y < gfx->virtualHeight; y++) {
        // Print a clean starting row decoration line
        printf("/* Row %03d */ \"", y);
        
        for (int x = 0; x < gfx->virtualWidth; x++) {
            // Fetch the clean color index using our fresh pget API
            int colorIndex = PR_PGet(x, y);
            
            // Map 0-31 indices seamlessly to clear 1-character hexadecimal letters
            char hexChar;
            if (colorIndex < 10) {
                hexChar = '0' + colorIndex;
            } else {
                hexChar = 'A' + (colorIndex - 10);
            }
            
            // Output character directly into the system stdout command stream pipeline
            printf("%c", hexChar);
        }
        
        // Close row string representation block with newline leap
        printf("\",\n");
    }
    
    printf("--- PICO-RAY OS | ACTIVE SOFTWARE VRAM MEMORY DUMP END ---\n\n");
}
