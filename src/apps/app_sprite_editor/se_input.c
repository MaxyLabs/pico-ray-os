#include "../../framework/framework.h"
#include "app_sprite_editor.h"

static SpriteEditorState *editorState = NULL;
static const SpriteEditorLayout *editorLayout = NULL;

static int Input_GetSpriteSize(void) {
    return SpriteEditorShared_GetSpriteSize(editorState);
}

static void Input_ClampViewport(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || editorLayout == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);
    int maxOffsetX = (stride > editorLayout->sheetW) ? (stride - editorLayout->sheetW) : 0;
    int maxOffsetY = (rows > editorLayout->sheetH) ? (rows - editorLayout->sheetH) : 0;

    editorState->sheetViewOffsetX = SpriteEditorShared_ClampInt(editorState->sheetViewOffsetX, 0, maxOffsetX);
    editorState->sheetViewOffsetY = SpriteEditorShared_ClampInt(editorState->sheetViewOffsetY, 0, maxOffsetY);
}

static bool Input_IsInsideSheet(MouseState mousePos) {
    if (editorLayout == NULL) return false;
    return mousePos.y - editorLayout->sheetY >= 0 && mousePos.y - editorLayout->sheetY < editorLayout->sheetH &&
           mousePos.x - editorLayout->sheetX >= 0 && mousePos.x - editorLayout->sheetX < editorLayout->sheetW;
}

static void Input_ClampSelectionToSheet(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);
    int spriteSize = Input_GetSpriteSize();

    int spriteMaxX = (stride / spriteSize) - 1;
    int spriteMaxY = (rows / spriteSize) - 1;

    if (spriteMaxX < 0) spriteMaxX = 0;
    if (spriteMaxY < 0) spriteMaxY = 0;

    editorState->selectedSpriteX = SpriteEditorShared_ClampInt(editorState->selectedSpriteX, 0, spriteMaxX);
    editorState->selectedSpriteY = SpriteEditorShared_ClampInt(editorState->selectedSpriteY, 0, spriteMaxY);
}

static void Input_EnsureSelectedVisible(void) {
    if (editorState == NULL || editorLayout == NULL) return;

    int spriteSize = Input_GetSpriteSize();
    int selectedPixelX = editorState->selectedSpriteX * spriteSize;
    int selectedPixelY = editorState->selectedSpriteY * spriteSize;

    if (selectedPixelX < editorState->sheetViewOffsetX) {
        editorState->sheetViewOffsetX -= spriteSize;
    } else if (selectedPixelX + spriteSize > editorState->sheetViewOffsetX + editorLayout->sheetW) {
        editorState->sheetViewOffsetX += spriteSize;
    }

    if (selectedPixelY < editorState->sheetViewOffsetY) {
        editorState->sheetViewOffsetY -= spriteSize;
    } else if (selectedPixelY + spriteSize > editorState->sheetViewOffsetY + editorLayout->sheetH) {
        editorState->sheetViewOffsetY += spriteSize;
    }

    Input_ClampViewport();
}

static void Input_MoveSelectedSprite(int deltaX, int deltaY) {
    if (editorState == NULL) return;
    editorState->selectedSpriteX += deltaX;
    editorState->selectedSpriteY += deltaY;
    Input_ClampSelectionToSheet();
    Input_EnsureSelectedVisible();
}

void SpriteEditorInput_SetSpriteSize(int spriteSize) {
    if (editorState == NULL) return;

    editorState->spriteSize = SpriteEditorShared_ClampInt(spriteSize, 1, SPRITE_EDITOR_MAX_SPRITE_SIZE);
    editorState->hoveredPixelX = -1;
    editorState->hoveredPixelY = -1;

    Input_ClampSelectionToSheet();
    Input_EnsureSelectedVisible();
}

static void Input_UpdateSheetDrag(MouseState mousePos) {
    if (editorState == NULL) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && Input_IsInsideSheet(mousePos)) {
        editorState->isSheetDragging = true;
        editorState->sheetDragLastMouseX = mousePos.x;
        editorState->sheetDragLastMouseY = mousePos.y;
    }

    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        editorState->isSheetDragging = false;
    }

    if (editorState->isSheetDragging) {
        int deltaX = mousePos.x - editorState->sheetDragLastMouseX;
        int deltaY = mousePos.y - editorState->sheetDragLastMouseY;

        editorState->sheetViewOffsetX -= deltaX;
        editorState->sheetViewOffsetY -= deltaY;

        editorState->sheetDragLastMouseX = mousePos.x;
        editorState->sheetDragLastMouseY = mousePos.y;

        Input_ClampViewport();
    }
}

void SpriteEditorInput_Init(SpriteEditorState *state, const SpriteEditorLayout *layout) {
    editorState = state;
    editorLayout = layout;
}

void SpriteEditorInput_UpdateShortcuts(void) {
    if (editorState == NULL || editorLayout == NULL) return;

    if (IsKeyPressed(KEY_G)) {
        editorState->isGridActive = !editorState->isGridActive;
    }

    // Arrow keys move selected sprite; viewport follows when crossing visible edges.
    if (IsKeyPressed(KEY_LEFT))  Input_MoveSelectedSprite(-1, 0);
    if (IsKeyPressed(KEY_RIGHT)) Input_MoveSelectedSprite(1, 0);
    if (IsKeyPressed(KEY_UP))    Input_MoveSelectedSprite(0, -1);
    if (IsKeyPressed(KEY_DOWN))  Input_MoveSelectedSprite(0, 1);

    Input_ClampViewport();
}

void SpriteEditorInput_UpdateDrawingBoard(MouseState mousePos) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || editorLayout == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int canvasInnerX = editorLayout->canvasX + 1;
    int canvasInnerY = editorLayout->canvasY + 1;
    int cellSize = SpriteEditorShared_GetCanvasCellSize(editorLayout, editorState);
    int canvasInnerW = cellSize * Input_GetSpriteSize();
    int canvasInnerH = cellSize * Input_GetSpriteSize();

    if (mousePos.x >= canvasInnerX && mousePos.x < canvasInnerX + canvasInnerW &&
        mousePos.y >= canvasInnerY && mousePos.y < canvasInnerY + canvasInnerH) {
        editorState->hoveredPixelX = (mousePos.x - canvasInnerX) / cellSize;
        editorState->hoveredPixelY = (mousePos.y - canvasInnerY) / cellSize;
    } else {
        editorState->hoveredPixelX = -1;
        editorState->hoveredPixelY = -1;
    }

    SpriteEditorTools_ApplyActiveTool();
}

void SpriteEditorInput_UpdatePaletteSelection(MouseState mousePos) {
    if (editorState == NULL || editorLayout == NULL) return;

    int x = editorLayout->paletteX;
    int y = editorLayout->paletteY;
    int csize = editorLayout->paletteColorSize + 1;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (mousePos.x >= x && mousePos.x < x + (16 * csize) && mousePos.y >= y && mousePos.y < y + csize) {
            editorState->activeColor = (mousePos.x - x) / csize;
        }

        int row2Y = y + csize;
        if (mousePos.x >= x && mousePos.x < x + (16 * csize) && mousePos.y >= row2Y && mousePos.y < row2Y + csize) {
            editorState->activeColor = 16 + ((mousePos.x - x) / csize);
        }
    }
}

void SpriteEditorInput_UpdateSpriteSheetSelection(MouseState mousePos) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || editorLayout == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    Input_UpdateSheetDrag(mousePos);

    if (editorState->isSheetDragging) {
        return;
    }

    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);
    int spriteSize = Input_GetSpriteSize();
    int spriteMaxX = (stride / spriteSize) - 1;
    int spriteMaxY = (rows / spriteSize) - 1;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (mousePos.y - editorLayout->sheetY >= 0 && mousePos.y - editorLayout->sheetY < editorLayout->sheetH &&
            mousePos.x - editorLayout->sheetX >= 0 && mousePos.x - editorLayout->sheetX < editorLayout->sheetW) {
            int localX = mousePos.x - editorLayout->sheetX;
            int localY = mousePos.y - editorLayout->sheetY;
            int sheetPixelX = editorState->sheetViewOffsetX + localX;
            int sheetPixelY = editorState->sheetViewOffsetY + localY;

            editorState->selectedSpriteX = SpriteEditorShared_ClampInt(sheetPixelX / spriteSize, 0, spriteMaxX);
            editorState->selectedSpriteY = SpriteEditorShared_ClampInt(sheetPixelY / spriteSize, 0, spriteMaxY);
            Input_EnsureSelectedVisible();
        }
    }
}
