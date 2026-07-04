#ifndef APP_SPRITE_EDITOR_RENDER_H
#define APP_SPRITE_EDITOR_RENDER_H

#include "../../framework/framework.h"
#include "app_sprite_editor.h"

void SpriteEditorRender_Init(SpriteEditorState *state, const SpriteEditorLayout *layout);
void SpriteEditorRender_DrawAll(Button *btnCopy, Button *btnPaste, Button *btnToolPencil, Button *btnToolBucket);

#endif // APP_SPRITE_EDITOR_RENDER_H
