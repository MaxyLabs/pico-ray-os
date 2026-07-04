#include <string.h>
#include "../framework.h"

// Instantiates a clean, standalone menu dropdown column element header
Menu PR_CreateMenu(const char *title) {
    Menu menu;
    menu.title = title;
    menu.optionCount = 0;
    menu.calculatedX = 0;
    menu.calculatedWidth = 0;
    return menu;
}

// Injects an interactive clickable command node row safely into a target dropdown container stack
void PR_AddMenuItem(Menu *menu, const char *text, int iconId, void (*callback)(void)) {
    // Safety guard clause: enforce the strict structural item row ceiling limit (Max 8)
    if (menu->optionCount < 8) {
        menu->options[menu->optionCount].text = text;
        menu->options[menu->optionCount].iconId = iconId;
        menu->options[menu->optionCount].onClick = callback; 
        menu->optionCount++;
    }
}

// Registers a finalized visual layout menu tab object directly onto the core operating system bar memory stack
void PR_RegisterApplicationMenu(Menu appMenu) {
    MenuSystem *menuSystem = PR_GetMenuSystem();

    if (menuSystem->totalActiveMenus < MAX_SYSTEM_MENUS) {
        menuSystem->systemMenus[menuSystem->totalActiveMenus] = appMenu;
        menuSystem->systemMenus[menuSystem->totalActiveMenus].title = appMenu.title;
        menuSystem->totalActiveMenus++;
    }
}

// Query hook scanning the system active stack to fetch a direct pointer reference to a specific tab by title
Menu* PR_GetRegisteredMenu(const char *title) {
    MenuSystem *menuSystem = PR_GetMenuSystem();

    for (int i = 0; i < menuSystem->totalActiveMenus; i++) {
        if (menuSystem->systemMenus[i].title != NULL && strcmp(menuSystem->systemMenus[i].title, title) == 0) {
            return &menuSystem->systemMenus[i];
        }
    }
    return NULL;
}

// --- AUTOMATED KERNEL DEFAULTS GENERATOR ---

void PR_UpdateMenuSystem(Menu *menus, int menuCount, int *activeMenuVar, MouseState mousePos) {
    if (menus == NULL || activeMenuVar == NULL || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    // 1. TOP BAR MAIN HEADERS CLICK CHECK (Y: 0 to 8 pixels)
    if (mousePos.y >= 0 && mousePos.y <= 8) {
        for (int i = 0; i < menuCount; i++) {
            if (mousePos.x >= menus[i].calculatedX && mousePos.x < menus[i].calculatedX + menus[i].calculatedWidth) {
                int targetMenu = i + 1;
                *activeMenuVar = (*activeMenuVar == targetMenu) ? 0 : targetMenu;
                return;
            }
        }
        *activeMenuVar = 0; 
        return;
    }

    // 2. DROPDOWN SUB-MENU CARD CLICKS TRACKING (EVENT DISPATCHING)
    if (*activeMenuVar > 0 && *activeMenuVar <= menuCount) {
        int currentIdx = *activeMenuVar - 1; 
        Menu *menu = &menus[currentIdx];

        int boxW = 85; 
        int boxH = menu->optionCount * 8 + 2;

        if (mousePos.x >= menu->calculatedX && mousePos.x < menu->calculatedX + boxW &&
            mousePos.y > 8 && mousePos.y <= 8 + boxH) {
            
            int clickedItem = (int)(mousePos.y - 9) / 8;
            
            if (clickedItem >= 0 && clickedItem < menu->optionCount) {
                // FETCH THE REGISTERED FUNCTION LINK
                void (*callback)(void) = menu->options[clickedItem].onClick;
                
                *activeMenuVar = 0; // Auto-collapse menu layout visually immediately
                
                // EVENT-DRIVEN FIRE: If the callback address is valid, trigger it instantly!
                if (callback != NULL) {
                    callback(); 
                }
                return;
            }
        }
        *activeMenuVar = 0; 
    }
}

// COMPONENT HELPER: Rasterizes the active dropdown option list card array onto VRAM
// This is called internally at the end of the menu system render pass if a tab is expanded
static void PR_DrawDropdownMenuCard(Menu *activeMenu, int winX, int winY, int winWidth) {
    if (activeMenu == NULL || activeMenu->optionCount == 0) return;

    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    ThemeRegistry theme = themeData[PR_GetActiveThemeId()];
    MouseState mousePos = PR_GetMousePosition(); // Fetch live physical cursor tracking parameters

    int rowHeight = 9; // Grid rows layout height matching retro typography size limits
    int cardHeight = activeMenu->optionCount * rowHeight + 2;

    // Render base dropdown card background block container box
    PR_RectFill(winX, winY, winWidth, cardHeight, theme.menuWindowBg);
    PR_Rect(winX, winY, winWidth, cardHeight, theme.menuWindowText); // Outer boundary stroke

    // Loop through compiled dropdown rows processing reactive hover highlights checks
    for (int j = 0; j < activeMenu->optionCount; j++) {
        int optionX = winX + 1;
        int optionY = winY + 1 + (j * rowHeight);
        int optionW = winWidth - 2;
        int optionH = rowHeight;

        // LIVE HOVER COLLISION RADAR DETECTION GATES
        bool isMouseOverRow = (mousePos.x >= optionX && mousePos.x < (optionX + optionW) &&
                               mousePos.y >= optionY && mousePos.y < (optionY + optionH));

        // Dynamically select targeted palette color indexes based on active interaction properties
        int bgCol   = isMouseOverRow ? theme.menuWindowHoverBg   : theme.menuWindowBg;
        int textCol = isMouseOverRow ? theme.menuWindowHoverText : theme.menuWindowText;

        // Repaint line slot row overlay plate immediately if cursor is hover active!
        if (isMouseOverRow) {
            PR_RectFill(optionX, optionY, optionW, optionH, bgCol);
        }

        // Render the option text label string safely aligned inside the calculated frame
        if (activeMenu->options[j].text != NULL) {
            PR_PrintPro(gfx->systemFontId, activeMenu->options[j].text, (float)(optionX + 4), (float)(optionY + 1), textCol);
        }
    }
}

// Renders the entire global operating system top bar and any active dropdown cards
void PR_DrawMenuSystem(Menu *menus, int menuCount, int activeMenuVar) {
    if (menus == NULL) return;
    
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    if (gfx == NULL || gfx->virtualVRAM.data == NULL) return;

    int menuWidth  = gfx->virtualWidth;
    int menuHeight = 8;

    // Fetch dynamic systemic context styling parameters from the active theme registry register
    int themeId = PR_GetActiveThemeId();
    ThemeRegistry theme = themeData[themeId];

    // Paint the top bar container bounding header strip smoothly via your unified grayscale hack!
    PR_RectFill(0, 0, menuWidth, menuHeight, theme.topBarBg);

    // Render the horizontal main top menu strip background layout
    PR_RectFill(0, 0, menuWidth, menuHeight, themeData[themeId].topBarBg);
    // PR_Line(0, 8, menuWidth, menuHeight, themeData[themeId].menuCardBorder);

    // Retrieve active cursor metrics directly to compute live responsive hover bounding checks
    MouseState mousePos = PR_GetMousePosition();

    // Loop through registered menu headers rasterizing titles and icons pixel-perfectly
    for (int i = 0; i < menuCount; i++) {
        if (menus[i].optionCount == 0) { continue; }

        // Bounding box tracking parameters for mouse hover matrix checks
        int xStart = menus[i].calculatedX;
        int xEnd   = xStart + menus[i].calculatedWidth;
        int yStart = 0;
        int yEnd   = 8; // Full strip height dimension

        // LIVE HOVER COLLISION RADAR DETECTION GATES
        bool isMouseHovering = (mousePos.x >= xStart && mousePos.x < xEnd && mousePos.y >= yStart && mousePos.y < yEnd);

        // Dynamically select target color registers matching the interaction state matrix
        int currentBgColor   = isMouseHovering ? theme.topBarHoverBg   : theme.topBarBg;
        int currentTextColor = isMouseHovering ? theme.topBarHoverText : theme.topBarText;

        // If mouse is hover active over this menu tab, repaint its bounding box anchor slot cleanly!
        if (isMouseHovering) {
            PR_RectFill(xStart, yStart, menus[i].calculatedWidth, yEnd, currentBgColor);
        }

        int textX = menus[i].calculatedX + 2;
        int textY = 1;
        
        if (menus[i].title[0] == 'o' && menus[i].title[1] == '\0') {
            // Render the pristine system Apple icon mask onto column zero
            PR_DrawIcon(ICON_APPLE, menus[i].calculatedX + 2, textY, currentTextColor);
        } else {
            // Render proportional typographic strings straight onto our software VRAM!
            PR_PrintPro(gfx->systemFontId, menus[i].title, (float)textX, (float)textY, currentTextColor);
        }
    }

    // RENDERING THE ACTIVE DROPDOWN ITEM LIST CARD
    // activeMenuVar is 1-based (0 means collapsed/none), so we verify bounds securely
    if (activeMenuVar > 0 && activeMenuVar <= menuCount) {
        int currentIdx = activeMenuVar - 1; // Convert back to standard 0-based array index
        Menu *menu = &menus[currentIdx];

        // Standard dropdown metrics configuration dimensions
        int boxW = 85;
        int boxH = menu->optionCount * 8 + 2;

        // Draw the solid bounding dropdown container card layout anchored to the calculated horizontal offset
        PR_RectFill(menu->calculatedX, 9, boxW, boxH, themeData[themeId].menuCardBg);
        PR_Rect(menu->calculatedX, 9, boxW, boxH, themeData[themeId].menuCardBorder);

        // Iterate through and render every single registered row option inside this active tab
        for (int i = 0; i < menu->optionCount; i++) {
            // Standard string row height calculation tracking (8 vertical pixel steps per entry slot)
            int itemY = 11 + (i * 8);

            // FIX: COMPUTE EXACT DISCRETE GEOMETRY BOUNDING BOX FOR HOVER DETECTION
            // The row boundary starts exactly inside the card borders (1px padding gap)
            int optionX = menu->calculatedX + 1;
            int optionY = 10 + (i * 8); // Synchronized with your itemY rendering grid baseline
            int optionW = boxW - 2;
            int optionH = 8; // 8 vertical pixels per option slot

            // LIVE DROPDOWN HOVER DETECTION GATE
            bool isMouseOverOption = (mousePos.x >= optionX && mousePos.x < (optionX + optionW) &&
                                      mousePos.y >= optionY && mousePos.y < (optionY + optionH));

            // Dynamically query target palette color registers from your active theme structural layout
            int bgCol   = isMouseOverOption ? theme.menuWindowHoverBg   : themeData[themeId].menuCardBg;
            int textCol = isMouseOverOption ? theme.menuWindowHoverText : themeData[themeId].menuCardText;

            // FIX: Repaint background slice plate immediately if option item row is hover active!
            if (isMouseOverOption) {
                PR_RectFill(optionX, optionY, optionW, optionH, bgCol);
            }

            int textOffsetX = menu->calculatedX + 4;

            // If the drop-down menu item possesses a valid registered graphical icon symbol
            if (menu->options[i].iconId >= 0) {
                // Render the 6x6 system icon aligned with the row layout using dynamic color tinting
                PR_DrawIcon(menu->options[i].iconId, menu->calculatedX + 4, itemY, textCol);
                textOffsetX += 8; // Shift text label slightly to the right to cleanly clear the icon bounding space
            }

            // Print the final selection text row option using the live active responsive color tokens!
            PR_PrintPro(gfx->systemFontId, menu->options[i].text, textOffsetX, itemY, textCol);
        }
    }
}
