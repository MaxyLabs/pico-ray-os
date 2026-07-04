#include <stdio.h>
#include <stdlib.h>
#include "../../framework/framework.h"
#include "app_sprite_editor.h"

static SpriteEditorState *editorState = NULL;

static int Tools_GetSpriteSize(void) { return SpriteEditorShared_GetSpriteSize(editorState); }

void SpriteEditorTools_Init(SpriteEditorState *state) {
    editorState = state;
}

void SpriteEditorTools_CommandCopy(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int spriteSize = Tools_GetSpriteSize();
    int stride = SpriteEditorShared_GetStride(cart);
    int spriteOffsetX = editorState->selectedSpriteX * spriteSize;
    int spriteOffsetY = editorState->selectedSpriteY * spriteSize;

    for (int y = 0; y < spriteSize; y++) {
        for (int x = 0; x < spriteSize; x++) {
            int memIndex = ((spriteOffsetY + y) * stride) + (spriteOffsetX + x);
            editorState->copyBuffer[y][x] = cart->spriteRAM[memIndex] & 0x1F;
        }
    }
}

void SpriteEditorTools_CommandPaste(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int spriteSize = Tools_GetSpriteSize();
    int stride = SpriteEditorShared_GetStride(cart);
    int spriteOffsetX = editorState->selectedSpriteX * spriteSize;
    int spriteOffsetY = editorState->selectedSpriteY * spriteSize;

    for (int y = 0; y < spriteSize; y++) {
        for (int x = 0; x < spriteSize; x++) {
            int memIndex = ((spriteOffsetY + y) * stride) + (spriteOffsetX + x);
            cart->spriteRAM[memIndex] = editorState->copyBuffer[y][x] & 0x1F;
        }
    }
}

static void Tools_Pencil(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int spriteSize = Tools_GetSpriteSize();
    unsigned char targetColorIndex = (unsigned char)editorState->activeColor;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            targetColorIndex = (unsigned char)editorState->activeColor;
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            targetColorIndex = 0;
        }

        int spriteOffsetX = editorState->selectedSpriteX * spriteSize;
        int spriteOffsetY = editorState->selectedSpriteY * spriteSize;

        int memX = spriteOffsetX + editorState->hoveredPixelX;
        int memY = spriteOffsetY + editorState->hoveredPixelY;

        cart->spriteRAM[(memY * stride) + memX] = targetColorIndex & 0x1F;
    }
}

static void Tools_FloodFill(int startX, int startY, unsigned char fillColor) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (editorState == NULL || cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int spriteSize = Tools_GetSpriteSize();

    int spriteOffsetX = editorState->selectedSpriteX * spriteSize;
    int spriteOffsetY = editorState->selectedSpriteY * spriteSize;

    int startMemX = spriteOffsetX + startX;
    int startMemY = spriteOffsetY + startY;

    unsigned char targetColor = cart->spriteRAM[(startMemY * stride) + startMemX] & 0x1F;
    if (targetColor == fillColor) return;

    int maxQueue = spriteSize * spriteSize;
    int *queueX = (int *)malloc((size_t)maxQueue * sizeof(int));
    int *queueY = (int *)malloc((size_t)maxQueue * sizeof(int));
    if (queueX == NULL || queueY == NULL) {
        free(queueX);
        free(queueY);
        return;
    }
    int head = 0;
    int tail = 0;

    queueX[tail] = startX;
    queueY[tail] = startY;
    tail++;

    cart->spriteRAM[(startMemY * stride) + startMemX] = fillColor & 0x1F;

    while (head < tail) {
        int cx = queueX[head];
        int cy = queueY[head];
        head++;

        int memX = spriteOffsetX + cx;
        int memY = spriteOffsetY + cy;

        if (cx > 0) {
            int checkIdx = (memY * stride) + (memX - 1);
            if ((cart->spriteRAM[checkIdx] & 0x1F) == targetColor && tail < maxQueue) {
                cart->spriteRAM[checkIdx] = fillColor & 0x1F;
                queueX[tail] = cx - 1;
                queueY[tail] = cy;
                tail++;
            }
        }

        if (cx < spriteSize - 1) {
            int checkIdx = (memY * stride) + (memX + 1);
            if ((cart->spriteRAM[checkIdx] & 0x1F) == targetColor && tail < maxQueue) {
                cart->spriteRAM[checkIdx] = fillColor & 0x1F;
                queueX[tail] = cx + 1;
                queueY[tail] = cy;
                tail++;
            }
        }

        if (cy > 0) {
            int checkIdx = ((memY - 1) * stride) + memX;
            if ((cart->spriteRAM[checkIdx] & 0x1F) == targetColor && tail < maxQueue) {
                cart->spriteRAM[checkIdx] = fillColor & 0x1F;
                queueX[tail] = cx;
                queueY[tail] = cy - 1;
                tail++;
            }
        }

        if (cy < spriteSize - 1) {
            int checkIdx = ((memY + 1) * stride) + memX;
            if ((cart->spriteRAM[checkIdx] & 0x1F) == targetColor && tail < maxQueue) {
                cart->spriteRAM[checkIdx] = fillColor & 0x1F;
                queueX[tail] = cx;
                queueY[tail] = cy + 1;
                tail++;
            }
        }
    }

    printf("PICO-RAY OS | SPRITE EDITOR | Bucket tool filled exactly %d pixels grid-aligned.\n", tail);
    free(queueX);
    free(queueY);
}

void SpriteEditorTools_ApplyActiveTool(void) {
    if (editorState == NULL) return;
    if (editorState->hoveredPixelX < 0 || editorState->hoveredPixelY < 0) return;

    if (editorState->activeToolId == TOOL_PENCIL) {
        Tools_Pencil();
    } else if (editorState->activeToolId == TOOL_BUCKET_FILL) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Tools_FloodFill(editorState->hoveredPixelX, editorState->hoveredPixelY, (unsigned char)editorState->activeColor);
        }
    }
}
