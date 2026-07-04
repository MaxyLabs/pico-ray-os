#include <stdio.h>
#include "../../framework/framework.h"
#include "app_sprite_editor.h"

static SpriteEditorState *editorState = NULL;
static const SpriteEditorLayout *editorLayout = NULL;

static int Render_GetSpriteSize(void) {
    return SpriteEditorShared_GetSpriteSize(editorState);
}

static int Render_GetSelectedSpriteId(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || cart == NULL || cart->spriteRAM == NULL) return 0;

    int stride = SpriteEditorShared_GetStride(cart);
    int spriteSize = Render_GetSpriteSize();
    int spritesPerRow = (spriteSize > 0) ? (stride / spriteSize) : 0;
    if (spritesPerRow <= 0) spritesPerRow = 1;

    return (editorState->selectedSpriteY * spritesPerRow) + editorState->selectedSpriteX;
}

void SpriteEditorRender_Init(SpriteEditorState *state, const SpriteEditorLayout *layout) {
    editorState = state;
    editorLayout = layout;
}

static void Render_DrawingBoard(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || editorLayout == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int spriteSize = Render_GetSpriteSize();
    int cellSize = SpriteEditorShared_GetCanvasCellSize(editorLayout, editorState);

    PR_RectFill(0, 0, 128, 128, editorLayout->bgWorkspaceColor);

    PR_RectFill(editorLayout->canvasX, editorLayout->canvasY, editorLayout->canvasW, editorLayout->canvasH, editorLayout->bgCanvasColor);
    PR_RectFill(editorLayout->canvasX + 1, editorLayout->canvasY + 1, editorLayout->canvasW - 2, editorLayout->canvasH - 2, editorLayout->bgCanvasGridColor);

    int spriteOffsetX = editorState->selectedSpriteX * spriteSize;
    int spriteOffsetY = editorState->selectedSpriteY * spriteSize;

    for (int y = 0; y < spriteSize; y++) {
        for (int x = 0; x < spriteSize; x++) {
            int memIndex = ((spriteOffsetY + y) * stride) + (spriteOffsetX + x);
            int targetColorIndex = (int)(cart->spriteRAM[memIndex] & 0x1F);

            int pW = editorState->isGridActive ? ((cellSize > 1) ? (cellSize - 1) : 1) : cellSize;
            int pH = editorState->isGridActive ? ((cellSize > 1) ? (cellSize - 1) : 1) : cellSize;

            if (!editorState->isGridActive) {
                int maxW = (editorLayout->canvasW - 2) - (x * cellSize);
                int maxH = (editorLayout->canvasH - 2) - (y * cellSize);
                if (pW > maxW) pW = maxW;
                if (pH > maxH) pH = maxH;
            }

            PR_RectFill(
                editorLayout->canvasX + 1 + (x * cellSize),
                editorLayout->canvasY + 1 + (y * cellSize),
                pW,
                pH,
                targetColorIndex
            );
        }
    }
}

static void Render_PaletteSelection(void) {
    if (editorState == NULL || editorLayout == NULL) return;

    int x = editorLayout->paletteX;
    int y = editorLayout->paletteY;
    int csize = editorLayout->paletteColorSize;

    for (int i = 0; i < 16; i++) {
        PR_RectFill(x + (i * (csize + 1)), y, csize, csize, i);
        PR_RectFill(x + (i * (csize + 1)), y + csize + 1, csize, csize, i + 16);
    }

    if (editorState->activeColor >= 0 && editorState->activeColor < 16) {
        PR_Rect(x + (editorState->activeColor * (csize + 1)) - 1, y - 1, csize + 2, csize + 2, editorLayout->paletteSelectorColor);
    } else if (editorState->activeColor >= 16 && editorState->activeColor < 32) {
        PR_Rect(x + ((editorState->activeColor - 16) * (csize + 1)) - 1, y + csize, csize + 2, csize + 2, editorLayout->paletteSelectorColor);
    }
}

static void Render_Buttons(Button *btnCopy, Button *btnPaste, Button *btnToolPencil, Button *btnToolBucket) {
    PR_DrawButton(btnCopy);
    PR_DrawButton(btnPaste);
    PR_DrawButton(btnToolPencil);
    PR_DrawButton(btnToolBucket);
}

static void Render_SpriteSheet(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || editorLayout == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);
    int spriteSize = Render_GetSpriteSize();
    int viewX = editorState->sheetViewOffsetX;
    int viewY = editorState->sheetViewOffsetY;

    int availableW = stride - viewX;
    int availableH = rows - viewY;
    int drawW = (editorLayout->sheetW < availableW) ? editorLayout->sheetW : availableW;
    int drawH = (editorLayout->sheetH < availableH) ? editorLayout->sheetH : availableH;

    if (drawW < 0) drawW = 0;
    if (drawH < 0) drawH = 0;

    for (int y = 0; y < drawH; y++) {
        for (int x = 0; x < drawW; x++) {
            int memIndex = ((viewY + y) * stride) + (viewX + x);
            int colorIndex = (int)(cart->spriteRAM[memIndex] & 0x1F);
            PR_PSet(x + editorLayout->sheetX, y + editorLayout->sheetY, colorIndex);
        }
    }

    int selectionScreenX = editorLayout->sheetX + (editorState->selectedSpriteX * spriteSize) - viewX;
    int selectionScreenY = editorLayout->sheetY + (editorState->selectedSpriteY * spriteSize) - viewY;
    if (selectionScreenX < editorLayout->sheetX + editorLayout->sheetW &&
        selectionScreenY < editorLayout->sheetY + editorLayout->sheetH &&
        selectionScreenX + spriteSize >= editorLayout->sheetX &&
        selectionScreenY + spriteSize >= editorLayout->sheetY) {
        PR_Rect(selectionScreenX - 1, selectionScreenY - 1, spriteSize + 2, spriteSize + 2, editorLayout->spriteSelectorColor);
    }

    // Edge hints show when there is additional off-screen sheet content.
    if (viewX > 0) {
        PR_Print("<", editorLayout->sheetX + 1, editorLayout->sheetY + (editorLayout->sheetH / 2) - 3, editorLayout->spriteSelectorColor);
    }
    if (viewX + drawW < stride) {
        PR_Print(">", editorLayout->sheetX + editorLayout->sheetW - 6, editorLayout->sheetY + (editorLayout->sheetH / 2) - 3, editorLayout->spriteSelectorColor);
    }
    if (viewY > 0) {
        PR_Print("^", editorLayout->sheetX + (editorLayout->sheetW / 2) - 2, editorLayout->sheetY + 1, editorLayout->spriteSelectorColor);
    }
    if (viewY + drawH < rows) {
        PR_Print("v", editorLayout->sheetX + (editorLayout->sheetW / 2) - 2, editorLayout->sheetY + editorLayout->sheetH - 7, editorLayout->spriteSelectorColor);
    }
}

static void Render_BottomStatusBar(void) {
    if (editorState == NULL || editorLayout == NULL) return;

    PR_RectFill(editorLayout->statusX, editorLayout->statusY, editorLayout->statusW, editorLayout->statusH, editorLayout->bgStatusBarColor);

    int selectedSpriteId = Render_GetSelectedSpriteId();
    int spriteSize = Render_GetSpriteSize();
    char statusText[96];
    snprintf(
        statusText,
        sizeof(statusText),
        "[%d] V:[%d,%d] C:[%d,%d]",
        selectedSpriteId,
        editorState->sheetViewOffsetX,
        editorState->sheetViewOffsetY,
        editorState->hoveredPixelX,
        editorState->hoveredPixelY        
    );

    PR_Print(statusText, editorLayout->statusX + 4, editorLayout->statusY + 1, P8_DARK_PURPLE);
}

void SpriteEditorRender_DrawAll(Button *btnCopy, Button *btnPaste, Button *btnToolPencil, Button *btnToolBucket) {
    Render_DrawingBoard();
    Render_PaletteSelection();
    Render_Buttons(btnCopy, btnPaste, btnToolPencil, btnToolBucket);
    Render_SpriteSheet();
    Render_BottomStatusBar();
}
