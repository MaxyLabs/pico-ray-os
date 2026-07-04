#ifndef APP_SPRITE_EDITOR_SHARED_H
#define APP_SPRITE_EDITOR_SHARED_H

#include "../../framework/framework.h"
#include "app_sprite_editor.h"

int SpriteEditorShared_GetStride(const CartridgeRAM *cart);
int SpriteEditorShared_GetRows(const CartridgeRAM *cart);
int SpriteEditorShared_GetSpriteSize(const SpriteEditorState *state);
int SpriteEditorShared_ClampInt(int value, int minValue, int maxValue);
int SpriteEditorShared_GetCanvasCellSize(const SpriteEditorLayout *layout, const SpriteEditorState *state);

#endif // APP_SPRITE_EDITOR_SHARED_H
