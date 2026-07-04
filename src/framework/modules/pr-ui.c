#include "../framework.h"
#include <math.h>

extern const FontRegistry fontData[FONT_COUNT];
extern const IconRegistry iconData[ICON_COUNT];

extern AppConfig currentAppConfig;

// --- RENDSZER-SZINTŰ UI MOTOR ---
Button PR_CreateButton(int x, int y, int w, int h, const char *text, int iconId, void (*callback)(void)) {
    Button btn = { x, y, w, h, text, iconId, false, false, callback };
    return btn;
}

// Frissíti a gomb állapotát és TRUE-val tér vissza, ha RÁKATTINTOTTAK
bool PR_UpdateButton(Button *btn, MouseState mousePos) {
    if (mousePos.x >= btn->x && mousePos.x < btn->x + btn->width &&
        mousePos.y >= btn->y && mousePos.y < btn->y + btn->height) {
        btn->isHovered = true;
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            btn->isPressed = true;
            
            // FIX: EVENT-DRIVEN AUTOMATION
            // If this button holds a valid callback function target, trigger it immediately upon click!
            if (btn->onClick != NULL) {
                btn->onClick();
            }
            return true; 
        }
    } else {
        btn->isHovered = false;
        btn->isPressed = false;
    }
    return false;
}

// Automatikus retro gombkirajzolás
void PR_DrawButton(Button *btn) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    int themeId = PR_GetActiveThemeId();

    // Alapszín kiválasztása: ha hover, legyen kicsit világosabb grey, ha pressed, sötét grey
    int bgColor = themeData[themeId].buttonBg;
    
    if (btn->isHovered) bgColor = themeData[themeId].buttonHover;
    if (btn->isPressed) bgColor = themeData[themeId].buttonPressed;

    // Gomb doboza és kerete
    PR_RectFill(btn->x, btn->y, btn->width, btn->height, bgColor);
    PR_Rect(btn->x, btn->y, btn->width, btn->height, themeData[themeId].windowBorder);

    int textOffsetX = btn->x + 4;
    
    // Ikon kirajzolása, ha van megadva
    if (btn->iconId >= 0) {
        PR_DrawIcon(btn->iconId, btn->x + 2, btn->y + (btn->height / 2 - 2), themeData[themeId].buttonIcon);
        textOffsetX += 8; // Eltoljuk a szöveget, hogy ne takarja az ikont
    }

    // Szöveg középre igazítása függőlegesen
    PR_PrintPro(gfx->systemFontId, btn->text, textOffsetX, btn->y + (btn->height / 2 - 2), themeData[themeId].buttonText);
}

// TODO
void PR_DrawBox(int boxX, int boxY, int boxW, int boxH, int colorBase, int colorBorder1, int colorBorder2) {
    if (colorBase > 0) PR_RectFill(boxX, boxY, boxW, boxH, colorBase);
    if (colorBase > 0) PR_Rect(boxX, boxY, boxW, boxH, colorBorder1);
    if (colorBase > 0) PR_Rect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, colorBorder2);
}
