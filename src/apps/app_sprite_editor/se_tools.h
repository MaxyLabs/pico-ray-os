#ifndef APP_SPRITE_EDITOR_TOOLS_H
#define APP_SPRITE_EDITOR_TOOLS_H

#include "app_sprite_editor.h"

void SpriteEditorTools_Init(SpriteEditorState *state);

void SpriteEditorTools_CommandCopy(void);
void SpriteEditorTools_CommandPaste(void);
void SpriteEditorTools_ApplyActiveTool(void);

#endif // APP_SPRITE_EDITOR_TOOLS_H
