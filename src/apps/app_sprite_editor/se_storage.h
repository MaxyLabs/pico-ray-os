#ifndef APP_SPRITE_EDITOR_STORAGE_H
#define APP_SPRITE_EDITOR_STORAGE_H

#include "app_sprite_editor.h"

void SpriteEditorStorage_Init(SpriteEditorState *state);

void SpriteEditorStorage_CommandOpenSheetDialog(void);
void SpriteEditorStorage_CommandSaveSheetDialog(void);
void SpriteEditorStorage_CommandLoadPNG(void);
void SpriteEditorStorage_CommandSavePNG(void);
void SpriteEditorStorage_CommandExportText(void);

#endif // APP_SPRITE_EDITOR_STORAGE_H
