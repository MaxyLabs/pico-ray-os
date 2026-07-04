#include "../../framework/framework.h"
#include "app_sprite_editor.h"

int SpriteEditorShared_GetStride(const CartridgeRAM *cart) {
    const CartridgeRAM *defaultCart = PR_GetDefaultCartridgeRAM();
    if (cart != NULL && cart->spriteAtlasColumns > 0) {
        return cart->spriteAtlasColumns;
    }
    return defaultCart->spriteAtlasColumns;
}

int SpriteEditorShared_GetRows(const CartridgeRAM *cart) {
    int stride = SpriteEditorShared_GetStride(cart);
    if (cart == NULL || stride <= 0) return 0;
    return cart->spriteRAMSize / stride;
}

int SpriteEditorShared_GetSpriteSize(const SpriteEditorState *state) {
    if (state == NULL || state->spriteSize <= 0) return 8;
    if (state->spriteSize > SPRITE_EDITOR_MAX_SPRITE_SIZE) {
        return SPRITE_EDITOR_MAX_SPRITE_SIZE;
    }
    return state->spriteSize;
}

int SpriteEditorShared_ClampInt(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

int SpriteEditorShared_GetCanvasCellSize(const SpriteEditorLayout *layout, const SpriteEditorState *state) {
    if (layout == NULL) return 1;

    int spriteSize = SpriteEditorShared_GetSpriteSize(state);
    if (spriteSize == 8) {
        return layout->pixelGridSize;
    }

    int innerW = layout->canvasW - 2;
    int innerH = layout->canvasH - 2;
    int cellW = (spriteSize > 0) ? (innerW / spriteSize) : 1;
    int cellH = (spriteSize > 0) ? (innerH / spriteSize) : 1;
    int cell = (cellW < cellH) ? cellW : cellH;
    if (cell < 1) cell = 1;
    return cell;
}
