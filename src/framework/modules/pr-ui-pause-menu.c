#include <stdlib.h>
#include <string.h>
#include "../framework.h"

// Forward function declaration
static void Action_ResumeCartridge(void);
static void Action_ResetCartridge(void);
static void Action_MuteVolume(void);
static void Action_ChangeVolumeDown(void);
static void Action_ChangeVolumeUp(void);

static void Action_ToggleMenuBar(void);
static void Action_ReturnToMainMenu(void);
static void Action_BuildOptionsMenu(void);

static int cachedVolumeBeforeMute;
static char footerText[32];
static char menubarText[16];
static char volumeText[16];

void PR_AppendPauseMenuItem(PauseMenuItem item);
void PR_InitPauseMenu(int initialCapacity);
bool PR_UpdatePauseMenu(MouseState mousePos);
void PR_DrawPauseMenu(void);
void PR_CleanupPauseMenu(void);

static void Action_ResumeCartridge(void) {
    KernelState *kernel = PR_GetKernelState();
    kernel->isPauseMenuOpen = false;
}

static void Action_ResetCartridge(void) {
    // Trigger the framework level re-bootstrap process here, 
    // or set a kernel state flag like kernel->shouldResetActiveApp = true;
    // so the main framework loop can handle the actual .Init() call safely!
    KernelState *kernel = PR_GetKernelState();
    kernel->shouldResetApp = true;

    printf("PICO-RAY OS | KERNEL | Cartridge lifecycle reset triggered via callback.\n");
}

static void Action_MuteVolume(void) {
    KernelState *kernel = PR_GetKernelState();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    kernel->isSystemVolumeMuted = !kernel->isSystemVolumeMuted;

    if (kernel->isSystemVolumeMuted == true) {
        cachedVolumeBeforeMute = kernel->systemVolume;
        kernel->systemVolume = 0;

        PR_StrlCpy(pauseMenu->items[kernel->pauseSelectedIndex].label, "VOLUME: [MUTED]", sizeof(pauseMenu->items[kernel->pauseSelectedIndex].label));

        printf("PICO-RAY OS | KERNEL | AUDIO: Hardware MUTED [Muted from %d%%]\n", cachedVolumeBeforeMute);
    } else {
        kernel->systemVolume = cachedVolumeBeforeMute;

        PR_StrFormat(volumeText, sizeof(volumeText), "VOLUME: %d%%", kernel->systemVolume);
        PR_StrlCpy(pauseMenu->items[kernel->pauseSelectedIndex].label, volumeText, sizeof(pauseMenu->items[kernel->pauseSelectedIndex].label));

        printf("PICO-RAY OS | KERNEL | AUDIO: Hardware UNMUTED straight back to %d%%\n", kernel->systemVolume);
    }
}

static void Action_ChangeVolumeDown(void) {
    KernelState *kernel = PR_GetKernelState();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    if (kernel->isSystemVolumeMuted == true) {
        kernel->isSystemVolumeMuted = false;
        kernel->systemVolume = cachedVolumeBeforeMute;
    } else {
        kernel->systemVolume -= 10;
        if (kernel->systemVolume < 0) { kernel->systemVolume = 0; }  // Lock strict 0% floor limit
    }

    PR_StrFormat(volumeText, sizeof(volumeText), "VOLUME: %d%%", kernel->systemVolume);
    PR_StrlCpy(pauseMenu->items[kernel->pauseSelectedIndex].label, volumeText, sizeof(pauseMenu->items[kernel->pauseSelectedIndex].label));

    printf("PICO-RAY OS | KERNEL | AUDIO: Master volume decreased down to %d%%\n", kernel->systemVolume);
}

static void Action_ChangeVolumeUp(void) {
    KernelState *kernel = PR_GetKernelState();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    if (kernel->isSystemVolumeMuted == true) {
        kernel->isSystemVolumeMuted = false;
        kernel->systemVolume = cachedVolumeBeforeMute;
    } else {
        kernel->systemVolume += 10;
        if (kernel->systemVolume > 100) { kernel->systemVolume = 100; }  // Lock strict 100% ceiling cap
    }

    PR_StrFormat(volumeText, sizeof(volumeText), "VOLUME: %d%%", kernel->systemVolume);
    PR_StrlCpy(pauseMenu->items[kernel->pauseSelectedIndex].label, volumeText, sizeof(pauseMenu->items[kernel->pauseSelectedIndex].label));

    printf("PICO-RAY OS | KERNEL | AUDIO: Master volume increased up to %d%%\n", kernel->systemVolume);
}

static void Action_ToggleMenuBar(void) {
    KernelState *kernel = PR_GetKernelState();
    MenuSystem *menuSystem = PR_GetMenuSystem();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    menuSystem->isMenuBarVisible = !menuSystem->isMenuBarVisible;
    
    PR_StrFormat(menubarText, sizeof(menubarText), "MENUBAR: %s", menuSystem->isMenuBarVisible ? "[ON]" : "[OFF]");
    PR_StrlCpy(pauseMenu->items[kernel->pauseSelectedIndex].label, menubarText, sizeof(pauseMenu->items[kernel->pauseSelectedIndex].label));
}

static void Action_ReturnToMainMenu(void) {
    KernelState *kernel = PR_GetKernelState();
    
    // Completely wipe out the current dynamic array sizing count tracker
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();
    pauseMenu->size = 0; 
    
    // Re-trigger the base initialization pass to load default main layout menu rows
    PR_InitPauseMenu(pauseMenu->capacity);
    
    kernel->pauseSelectedIndex = 0;
}

static void Action_BuildOptionsMenu(void) {
    KernelState *kernel = PR_GetKernelState();
    MenuSystem *menuSystem = PR_GetMenuSystem();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    // Reset size count to 0 so we can populate completely fresh string element rows
    pauseMenu->size = 0;
    kernel->pauseSelectedIndex = 0; // Guard focus boundary reset for safety

    // MenuBar
    PauseMenuItem menubarItem = {
        .label = {0},
        .action = Action_ToggleMenuBar,
        .actionLeft = Action_ToggleMenuBar,
        .actionRight = Action_ToggleMenuBar,
        .luaCallbackRef = -1
    };

    PR_StrFormat(menubarItem.label, sizeof(menubarItem.label), "MENUBAR: %s", menuSystem->isMenuBarVisible ? "[ON]" : "[OFF]");

    // Back
    PauseMenuItem backItem = {
        .label = "BACK",
        .action = Action_ReturnToMainMenu,
        .luaCallbackRef = -1
    };

    PR_AppendPauseMenuItem(menubarItem);
    PR_AppendPauseMenuItem(backItem);
}

// Appends a new interactive line row with automatic realloc padding safeguards
void PR_AppendPauseMenuItem(PauseMenuItem item) {
    PauseMenuSystem *menu = PR_GetPauseMenu();

    // Safeguard lazy-init setup: if elements table is missing, force an emergency allocation pass
    if (menu->items == NULL) {
        PR_InitPauseMenu(5);
    }

    // Resizing gate: If capacity limit is hit, double the memory footprint
    if (menu->size >= menu->capacity) {
        menu->capacity *= 2;
        PauseMenuItem *temp = (PauseMenuItem*)realloc(menu->items, menu->capacity * sizeof(PauseMenuItem));
        if (temp != NULL) {
            menu->items = temp;
        } else {
            printf("PICO-RAY OS | KERNEL | ERROR: Failed to reallocate memory space for dynamic pause menu.\n");
            return;
        }
    }

    // Copy data smoothly into the structural matrix block slot
    menu->items[menu->size++] = item;
}

// Safely initializes the dynamic pause menu tracking matrix context
void PR_InitPauseMenu(int initialCapacity) {
    KernelState *kernel = PR_GetKernelState();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    pauseMenu->size = 0;
    pauseMenu->capacity = initialCapacity;
    pauseMenu->items = (PauseMenuItem*)malloc(initialCapacity * sizeof(PauseMenuItem));

    if (pauseMenu->items == NULL) {
        printf("PICO-RAY OS | KERNEL | ERROR: Critical memory allocation failure for Pause Menu!\n");
        return;
    }
    
    // Resume
    // NULL action handles core closing
    PauseMenuItem resumeItem = {
        .label = "RESUME",
        .action = Action_ResumeCartridge,
        .luaCallbackRef = -1
    };

    // Reset
    PauseMenuItem resetItem = {
        .label = "RESET CART",
        .action = Action_ResetCartridge,
        .luaCallbackRef = -1
    };

    // Volume
    PauseMenuItem volumeItem = {
        .label = {0},
        .action = Action_MuteVolume,
        .actionLeft = Action_ChangeVolumeDown,
        .actionRight = Action_ChangeVolumeUp,
        .luaCallbackRef = -1
    };

    PR_StrFormat(volumeText, sizeof(volumeText), "VOLUME: %d%%", kernel->systemVolume);
    PR_StrlCpy(volumeItem.label, volumeText, sizeof(volumeItem.label));

    // Options
    PauseMenuItem optionsItem = {
        .label = "OPTIONS",
        .action = Action_BuildOptionsMenu,
        .luaCallbackRef = -1
    };

    // Return Home
    PauseMenuItem returnHomeItem = {
        .label = "RETURN HOME",
        .action = PR_Callback_CloseApp,
        .luaCallbackRef = -1
    };

    PR_AppendPauseMenuItem(resumeItem);
    PR_AppendPauseMenuItem(resetItem);
    PR_AppendPauseMenuItem(volumeItem);
    PR_AppendPauseMenuItem(optionsItem);
    PR_AppendPauseMenuItem(returnHomeItem);
}

bool PR_UpdatePauseMenu(MouseState mousePos) {
    KernelState *kernel = PR_GetKernelState();
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();
    AppConfig appConfig = PR_GetActiveAppConfig();

    // Persistent cache variable to store the volume level right before triggering MUTE
    static int cachedVolumeBeforeMute = 75;

    // Disable the PauseMenu when the active app is: Home
    if (kernel->isAppLoaded && strcmp(appConfig.name, "Home") != 0) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            kernel->isPauseMenuOpen = !kernel->isPauseMenuOpen;

            if (kernel->isPauseMenuOpen) {
                kernel->pauseMode = PAUSE_MODE_MAIN;  // Reset pause mode
                kernel->pauseSelectedIndex = 0;  // Reset pause selector
            }
        }
    }
   
    if (kernel->isPauseMenuOpen) {
        int pauseMenuTotalItems = pauseMenu->size;

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            if (kernel->pauseSelectedIndex > 0) kernel->pauseSelectedIndex--;
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            if (kernel->pauseSelectedIndex < pauseMenuTotalItems - 1) kernel->pauseSelectedIndex++;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (pauseMenu->items[kernel->pauseSelectedIndex].action != NULL) {
                pauseMenu->items[kernel->pauseSelectedIndex].action();
            }
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            if (pauseMenu->items[kernel->pauseSelectedIndex].actionLeft != NULL) {
                pauseMenu->items[kernel->pauseSelectedIndex].actionLeft();
            }
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            if (pauseMenu->items[kernel->pauseSelectedIndex].actionRight != NULL) {
                pauseMenu->items[kernel->pauseSelectedIndex].actionRight();
            }
        }

        /*


        // Handle menu selection inputs row by row (Vertical navigation)
            


        // HORIZONTAL CONTROLLER INTERCEPTOR FOR VOLUME SLIDER ROW!
        if (kernel->pauseSelectedIndex == 2) {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                kernel->systemVolume -= 10;
                if (kernel->systemVolume < 0) kernel->systemVolume = 0; // Lock strict 0% floor limit
                printf("PICO-RAY OS | AUDIO: Master volume decreased down to %d%%\n", kernel->systemVolume);
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                kernel->systemVolume += 10;
                if (kernel->systemVolume > 100) kernel->systemVolume = 100; // Lock strict 100% ceiling cap
                printf("PICO-RAY OS | AUDIO: Master volume increased up to %d%%\n", kernel->systemVolume);
            }
        }

        // Action activation pass via ENTER/SPACE confirmation keystrokes
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (kernel->pauseSelectedIndex == 0) { // RESUME
                kernel->isPauseMenuOpen = false;
            }
            else if (kernel->pauseSelectedIndex == 1) { // RESET CART
                kernel->isPauseMenuOpen = false;
                if (activeApp.Init != NULL) activeApp.Init(); // Re-bootstrap application life-cycle
            }
            else if (kernel->pauseSelectedIndex == 2) { 
                // FIX: INSTANT TOGGLE MUTE / UNMUTE PIPELINE!
                // If the system is currently playing audio, mute it and cache the previous level.
                // If already muted, restore the precise percentage instantly!
                if (kernel->systemVolume > 0) {
                    cachedVolumeBeforeMute = kernel->systemVolume; // Lock current volume to cache
                    kernel->systemVolume = 0;                      // Set instantly to pure silence
                    printf("PICO-RAY OS | AUDIO: Hardware MUTED [Muted from %d%%]\n", cachedVolumeBeforeMute);
                } else {
                    // Restore from cache, ensuring we don't unmute back into a 0% loop trap
                    kernel->systemVolume = (cachedVolumeBeforeMute > 0) ? cachedVolumeBeforeMute : 75;
                    printf("PICO-RAY OS | AUDIO: Hardware UNMUTED straight back to %d%%\n", kernel->systemVolume);
                }
            }
            else if (kernel->pauseSelectedIndex == 3) {
                // CLOSE APP / RETURN HOME
                kernel->isPauseMenuOpen = false;
                PR_Callback_CloseApp(); // Force return channel back to desktop space
            }
        }
        */

        return false;  // Blocks input: Freezes further app logical updates
    } else {
        return true;
    }
}

void PR_DrawPauseMenu(void) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    KernelState *kernel = PR_GetKernelState();
    PauseMenuSystem *menu = PR_GetPauseMenu();
    ThemeRegistry theme = themeData[PR_GetActiveThemeId()];

    int winX, winY, winW, winH;
    int titleX, lengthTitle;
    int rowX, rowY, rowOffset;
    int colorPauseMenuText;

    // Pause Menu window size
    winX = 16;
    winY = 16;
    winW = gfx->virtualWidth  - (2 * winX);
    winH = gfx->virtualHeight - (2 * winY);

    // Fill the screen
    PR_RectFill(0, 0, gfx->virtualWidth, gfx->virtualHeight, theme.pauseMenuFill);

    // Draw Pause Menu window
    PR_RectFill(winX, winY, winW, winH, theme.pauseMenuBg);

    // Header plate row accent banner
    lengthTitle = PR_GetStringWidthByPixel(gfx->systemFontId, "SYSTEM PAUSED");
    titleX = winX + ((winW - lengthTitle) / 2);

    PR_RectFill(winX, winY, winW, 8, theme.pauseMenuTitleBg);
    PR_PrintPro(gfx->systemFontId, "SYSTEM PAUSED", titleX, winY + 2, theme.pauseMenuTitleText);

    for (int i = 0; i < menu->size; i++) {
        // int color = (pauseSelectedIndex == i) ? P8_GREEN : P8_WHITE;
        rowX = winX + 16;
        rowY = winY + 16;
        rowOffset = i * 10;

        if ( i == kernel->pauseSelectedIndex) {
            colorPauseMenuText = theme.pauseMenuSelectedText;
            PR_RectFill(winX + 8, rowY + rowOffset - 2, winW - 16, 10, theme.pauseMenuSelectedBg);
            PR_PrintPro(gfx->systemFontId, ">", winX + 8, rowY + rowOffset, theme.pauseMenuSelectedText);

        } else {
            colorPauseMenuText = theme.pauseMenuText;
        }

        PR_PrintPro(gfx->systemFontId, menu->items[i].label, rowX, rowY + rowOffset, colorPauseMenuText);
    }

    // const char *menuStrings[] = { "RESUME GAME", "RESET CART", "VOLUME", "RETURN HOME" };
    /*
    for (int i = 0; i < 4; i++) {
        int rowY = winY + 18 + (i * 12);
        int rowW = winW - 12;
        int rowX = winX + 6;
        // Live responsive text highlight tracking selectors hooks
        if (i == kernel->pauseSelectedIndex) {
            // Active hover line selection row indicator box
            PR_RectFill(rowX, rowY - 1, rowW, 9, P8_BLUE);
            
            // Add a micro selection dot indicator symbol
            PR_PrintPro(gfx->systemFontId, ">", rowX + 2, rowY, P8_YELLOW);
            
            if (i == 2) {
                char volText[16]; sprintf(volText, "VOLUME: %d%%", kernel->systemVolume);
                PR_PrintPro(gfx->systemFontId, volText, rowX + 10, rowY, P8_WHITE);
            } else {
                PR_PrintPro(gfx->systemFontId, menuStrings[i], rowX + 10, rowY, P8_WHITE);
            }
        } else {
            if (i == 2) {
                char volText[16]; sprintf(volText, "VOLUME: %d%%", kernel->systemVolume);
                PR_PrintPro(gfx->systemFontId, volText, rowX + 10, rowY, P8_DARK_GREY);
            } else {
                PR_PrintPro(gfx->systemFontId, menuStrings[i], rowX + 10, rowY, P8_DARK_GREY);
            }
        }
    }
    */

    // Micro branding line row layout footer decor strip



    PR_StrFormat(footerText, sizeof(footerText), "%s: %s", PICO_RAY_OS_NAME, PICO_RAY_OS_VER);

    rowX = winX + 8;
    rowY = winY + winH - 10;
    PR_Line(rowX, rowY, winX + winW - 8, rowY, theme.pauseMenuText);

    rowY = winY + winH - 8;
    PR_PrintPro(gfx->systemFontId, footerText, rowX, rowY, theme.pauseMenuText);

    // Draw Pause Menu window border
    PR_Rect(winX, winY, winW, winH, theme.pauseMenuBorder);
}

// Clean up memory on system shutdown to guarantee zero leaks
void PR_CleanupPauseMenu(void) {
    PauseMenuSystem *pauseMenu = PR_GetPauseMenu();

    if (pauseMenu->items != NULL) {
        free(pauseMenu->items);
        pauseMenu->items = NULL;
    }
}
