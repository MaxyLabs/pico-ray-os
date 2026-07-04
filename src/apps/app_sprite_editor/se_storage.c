#include <stdio.h>
#include <string.h>
#include "../../framework/framework.h"
#include "app_sprite_editor.h"

static SpriteEditorState *editorState = NULL;

static int Storage_GetIndex(const CartridgeRAM *cart, int x, int y) {
    return (y * SpriteEditorShared_GetStride(cart)) + x;
}

static char Storage_ColorToToken(unsigned char colorIdx) {
    unsigned char clamped = colorIdx & 0x1F;
    if (clamped == 0) return '.';
    if (clamped < 10) return (char)('0' + clamped);
    if (clamped < 16) return (char)('A' + (clamped - 10));
    return (char)('G' + (clamped - 16));
}

void SpriteEditorStorage_Init(SpriteEditorState *state) {
    editorState = state;
}

static void Callback_OnOpenSheetConfirmed(const char *filePath) {
    if (filePath == NULL || filePath[0] == '\0') return;

    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL) return;

    printf("PICO-RAY OS | SPRITE EDITOR | Slicing file sheet matrix cleanly...\n");

    if (!PR_LoadRayCartridge(filePath, cart)) {
        printf("PICO-RAY OS | SPRITE EDITOR | ERROR: Failed to parse asset file: %s\n", filePath);
        return;
    }

    cart->spriteRAMIndex = cart->spriteAtlasColumns * cart->spriteAtlasRows;

    if (editorState != NULL) {
        editorState->activeColor = 8;
        editorState->selectedSpriteX = 0;
        editorState->selectedSpriteY = 0;
    }

    printf("PICO-RAY OS | SPRITE EDITOR | Viewport locked and synchronized.\n");
}

void SpriteEditorStorage_CommandOpenSheetDialog(void) {
    PR_OpenFileDialog("carts", ".ray", Callback_OnOpenSheetConfirmed);
}

static void Callback_OnSaveSheetConfirmed(const char *filePath) {
    if (filePath == NULL || filePath[0] == '\0') return;

    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        printf("PICO-RAY OS | SPRITE EDITOR | ERROR: Cannot open file for writing: %s\n", filePath);
        return;
    }

    fprintf(file, "pico-ray cartridge\n");
    fprintf(file, "__meta__\n");

    CartridgeMeta *meta = PR_GetCartridgeMeta();
    if (meta != NULL && strlen(meta->name) > 0) {
        fprintf(file, "name=%s\n", meta->name);
        fprintf(file, "author=%s\n", meta->author);
        fprintf(file, "version=%s\n", meta->version);
        fprintf(file, "license=%s\n", meta->license);
        fprintf(file, "mode=%s\n", meta->mode);
    } else {
        fprintf(file, "name=Edited Sprite Sheet\n");
        fprintf(file, "author=PICO-RAY OS Editor\n");
        fprintf(file, "version=0.1.0");
        fprintf(file, "license=MIT");
        fprintf(file, "mode=pico8\n");
    }

    fprintf(file, "__gfx__\n");

    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart != NULL && cart->spriteRAM != NULL) {
        int width = SpriteEditorShared_GetStride(cart);
        int totalPixels = cart->spriteRAMSize;

        for (int i = 0; i < totalPixels; i++) {
            unsigned char colorIdx = cart->spriteRAM[i] & 0x1F;
            fputc(Storage_ColorToToken(colorIdx), file);

            if ((i + 1) % width == 0) {
                fputc('\n', file);
            }
        }
    }

    fprintf(file, "__map__\n");
    fprintf(file, "__lua__\n");

    fclose(file);
    printf("PICO-RAY OS | SPRITE EDITOR | Save transaction finished successfully.\n");
}

void SpriteEditorStorage_CommandSaveSheetDialog(void) {
    PR_SaveFileDialog("carts", ".ray", Callback_OnSaveSheetConfirmed);
}

static void ExportToTextFormat(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->spriteRAM == NULL) return;
    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);

    FILE *f = fopen("spritesheet_gfx.txt", "w");
    if (f == NULL) return;

    fprintf(f, "{\n");
    for (int sy = 0; sy < rows; sy++) {
        fprintf(f, "    \"");
        for (int sx = 0; sx < stride; sx++) {
            int memIndex = Storage_GetIndex(cart, sx, sy);
            unsigned char colorIndex = cart->spriteRAM[memIndex] & 0x1F;
            fprintf(f, "%c", Storage_ColorToToken(colorIndex));
        }
        if (sy == rows - 1) fprintf(f, "\"\n");
        else fprintf(f, "\",\n");
    }
    fprintf(f, "};\n");
    fclose(f);
}

void SpriteEditorStorage_CommandLoadPNG(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);

    if (FileExists("spritesheet.png")) {
        Image img = LoadImage("spritesheet.png");
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

        int scanW = (img.width > stride) ? stride : img.width;
        int scanH = (img.height > rows) ? rows : img.height;

        for (int y = 0; y < scanH; y++) {
            for (int x = 0; x < scanW; x++) {
                Color c = GetImageColor(img, x, y);
                cart->spriteRAM[Storage_GetIndex(cart, x, y)] = (unsigned char)GetPicoColorFromColor(c);
            }
        }
        UnloadImage(img);
    }
}

void SpriteEditorStorage_CommandSavePNG(void) {
    CartridgeRAM *cart = PR_GetCartridgeRAM();
    if (cart == NULL || cart->spriteRAM == NULL) return;

    int stride = SpriteEditorShared_GetStride(cart);
    int rows = SpriteEditorShared_GetRows(cart);

    Image img = GenImageColor(stride, rows, BLANK);
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < stride; x++) {
            int colorIndex = cart->spriteRAM[Storage_GetIndex(cart, x, y)] & 0x1F;
            ImageDrawPixel(&img, x, y, PR_GetColor(colorIndex));
        }
    }
    ExportImage(img, "spritesheet.png");
    UnloadImage(img);
}

void SpriteEditorStorage_CommandExportText(void) {
    ExportToTextFormat();
}
