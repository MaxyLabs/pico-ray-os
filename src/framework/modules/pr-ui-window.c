#include <string.h>
#include "../framework.h"

void PR_DrawDebugWindow(void) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    MouseState mousePos = PR_GetMousePosition();
    
    PR_RectFill(0, 120, 128, 8, P8_ORANGE);
    PR_PrintPro(gfx->systemFontId, TextFormat("Mouse X:%d Y:%d", mousePos.x, mousePos.y), 4, 121, P8_BLACK);
}

// --- Adaptive System About Window Interface ---
void PR_DrawAboutWindow(void) {
    // FIX 1: STREAM METRICS FROM THE UNIFIED CONFIG PACK
    AppConfig config = PR_GetActiveAppConfig();
    GraphicsSystem *gfx = PR_GetGraphicsSystem();

    int midX = gfx->virtualWidth / 2;
    int midY = gfx->virtualHeight / 2;

    // Slightly grew height from 76 to 88 to comfortably frame author & license rows layout
    int boxW = 120;
    int boxH = 88; 
    int boxX = midX - (boxW / 2);
    int boxY = midY - (boxH / 2);

    // Dim the background using semi-transparent overlay cleanly
    DrawRectangle(0, 0, gfx->virtualWidth, gfx->virtualHeight, (Color){ 0, 0, 0, 170 });

    // Draw the main dialog window frame with your classic retro double border
    PR_RectFill(boxX, boxY, boxW, boxH, P8_LIGHT_GREY);
    PR_Rect(boxX, boxY, boxW, boxH, P8_BLACK);
    PR_Rect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, P8_WHITE);

    // RENDER ACTIVE APP INTEGRATED IDENTIFIERS (Read dynamically from unified configuration data)
    if (config.iconId >= 0) {
        PR_DrawIcon(config.iconId, boxX + 6, boxY + 6, P8_DARK_BLUE);
    }
    
    // FIX 2: ALIGNED TO UNIFIED FIELD NAMES (.name instead of appName)
    PR_PrintPro(FONT_PICORAY, config.name, boxX + 16, boxY + 6, P8_BLACK);
    PR_PrintPro(FONT_PICORAY, TextFormat("Version: %s", config.version), boxX + 16, boxY + 14, P8_DARK_GREY);
    
    PR_Line(boxX + 6, boxY + 23, boxX + boxW - 7, boxY + 23, P8_DARK_GREY);

    // RENDER METADATA LABELS USING OUR AUTOMATIC MULTI-LINE NEWLINE TRICK
    PR_PrintPro(FONT_PICORAY, "Author:", boxX + 6, boxY + 28, P8_DARK_GREY);
    PR_PrintPro(FONT_PICORAY, config.author, boxX + 42, boxY + 28, P8_BLACK); // Expands downwards safely if \n is inside!

    PR_PrintPro(FONT_PICORAY, "License:", boxX + 6, boxY + 48, P8_DARK_GREY);
    PR_PrintPro(FONT_PICORAY, config.license, boxX + 42, boxY + 48, P8_GREEN);

    // RENDER THE INTERACTIVE OK DISMISSAL NYOMÓGOMB
    int btnW = 24;
    int btnH = 9;
    int btnX = midX - (btnW / 2);
    int btnY = boxY + boxH - 13;

    // Create the button object mapping it directly to your internal static closure function target
    Button btnOK = PR_CreateButton(btnX, btnY, btnW, btnH, "OK", -1, PR_Callback_CloseAboutWindow);
    
    // Fetch live integer coordinates from your clean query API channel
    MouseState mousePos = PR_GetMousePosition();
    
    // Tick and render the button object simultaneously inside the modal loop!
    PR_UpdateButton(&btnOK, mousePos);
    PR_DrawButton(&btnOK);
}

// --- Adaptive System Information Window Interface ---
void PR_DrawSystemInfoWindow(void) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    int currentPaletteId = PR_GetActivePaletteId();

    int midX = gfx->virtualWidth / 2;
    int midY = gfx->virtualHeight / 2;

    int boxW = 120;
    int boxH = 76; // Using your tight 76px layout footprint comfortably
    int boxX = midX - (boxW / 2);
    int boxY = midY - (boxH / 2);

    // Dim the background using semi-transparent overlay
    DrawRectangle(0, 0, gfx->virtualWidth, gfx->virtualHeight, (Color){ 0, 0, 0, 170 });

    // Draw the main dialog window frame with your classic retro double border
    PR_RectFill(boxX, boxY, boxW, boxH, P8_LIGHT_GREY);
    PR_Rect(boxX, boxY, boxW, boxH, P8_BLACK);
    PR_Rect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, P8_WHITE);

    // RENDER BASE PLATFORM LAYER (PICO-RAY OS Branding Details)
    PR_DrawIcon(ICON_PICORAY, boxX + 6, boxY + 6, P8_RED);
    PR_PrintPro(FONT_PICORAY, PICO_RAY_OS_NAME, boxX + 16, boxY + 6, P8_BLACK);
    PR_PrintPro(FONT_PICORAY, TextFormat("Kernel: %s", PICO_RAY_OS_VER), boxX + 16, boxY + 14, P8_DARK_GREY); // Dynamic active folder check marker!
    
    PR_Line(boxX + 6, boxY + 23, boxX + boxW - 7, boxY + 23, P8_DARK_GREY);

    // LIVE HARDWARE METRICS (Read dynamic name string straight from paletteData array)
    PR_PrintPro(FONT_PICORAY, TextFormat("PALETTE: %s", paletteData[currentPaletteId].paletteName), boxX + 6, boxY + 28, P8_DARK_PURPLE);

    // SYSTEM UNDERLAYERS ATTRIBUTION RECORDS
    PR_PrintPro(FONT_PICORAY, "Powered by Raylib, Lua", boxX + 6, boxY + 40, P8_DARK_GREY);
    PR_PrintPro(FONT_PICORAY, "Inspired by PICO-8, Pyxel", boxX + 6, boxY + 48, P8_DARK_GREY);

    // RENDER THE INTERACTIVE OK DISMISSAL NYOMÓGOMB
    int btnW = 24;
    int btnH = 9;
    int btnX = midX - (btnW / 2);
    int btnY = boxY + boxH - 13;

    // Assumes you have mapped a clean close callback target like 'PR_Callback_CloseSystemInfoWindow'
    Button btnOK = PR_CreateButton(btnX, btnY, btnW, btnH, "OK", -1, PR_Callback_CloseSystemInfoWindow);
    
    MouseState mousePos = PR_GetMousePosition();
    
    PR_UpdateButton(&btnOK, mousePos);
    PR_DrawButton(&btnOK);
}
