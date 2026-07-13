#include <string.h>
#include <stdio.h>
#include "../framework.h"

static Button btnDialogOK;
static Button btnDialogCancel;

// Formats a clean unified relative path string securely across core triggers
static char staticPathBuffer[256];

// Standard alphabetic string comparison helper callback targeted for qsort pipeline stream
static int CompareFileNames(const void *a, const void *b) {
    const char *strA = *(const char **)a;
    const char *strB = *(const char **)b;
    return strcmp(GetFileName(strA), GetFileName(strB)); // Bypasses folder paths sorting cleanly
}

// Internal initializer core helper encapsulating shared directory scanning logic
static void InitBaseFileDialog(const char *folder, const char *extension, FileDialogMode mode, void (*callback)(const char *filePath)) {
    if (callback == NULL) return;

    FileDialogState *dialogState = PR_GetFileDialogState();

    // 1. Wipe and re-align our persistent state container register completely
    memset(dialogState, 0, sizeof(FileDialogState));
    dialogState->isOpen = true;
    dialogState->mode = mode;
    dialogState->onFileAction = callback;
    dialogState->selectedIndex = -1;
    
    if (extension != NULL) {
        strncpy(dialogState->extensionFilter, extension, 15);
    }

    // 2. Secure Raylib core file system directory scan probe
    FilePathList fileList = LoadDirectoryFiles(folder != NULL ? folder : ".");

    if (fileList.count > 1) {
        qsort(fileList.paths, fileList.count, sizeof(char *), CompareFileNames);
    }
    
    for (unsigned int i = 0; i < fileList.count && dialogState->fileCount < MAX_FILE_DIALOG_FILES; i++) {
        const char *name = fileList.paths[i];
        
        // Filter by file extension if requested by the application module
        if (dialogState->extensionFilter[0] == '\0' || IsFileExtension(name, dialogState->extensionFilter)) {
            const char *fileNameOnly = GetFileName(name);
            strncpy(dialogState->files[dialogState->fileCount], fileNameOnly, 63);
            dialogState->fileCount++;
        }
    }
    UnloadDirectoryFiles(fileList); // Flush temporary Raylib memory footprint

    // 3. Instantiate interactive control actions button tokens mapping layout rows
    btnDialogCancel = PR_CreateButton(12,  104, 48, 10, "CANCEL", -1, NULL);
    
    // Adapt main action confirmation string badge dynamically matching operational mode
    const char *actionBadge = (mode == FILE_DIALOG_SAVE) ? "SAVE" : "OPEN";
    btnDialogOK = PR_CreateButton(68,  104, 48, 10, actionBadge, -1, NULL);
}

// THE EXPOSED OPEN PIPELINE GATEWAY
void PR_OpenFileDialog(const char *folder, const char *extension, void (*callback)(const char *filePath)) {
    InitBaseFileDialog(folder, extension, FILE_DIALOG_OPEN, callback);
}

// THE EXPOSED SAVE PIPELINE GATEWAY
void PR_SaveFileDialog(const char *folder, const char *extension, void (*callback)(const char *filePath)) {
    FileDialogState *dialogState = PR_GetFileDialogState();

    InitBaseFileDialog(folder, extension, FILE_DIALOG_SAVE, callback);
    
    // Seed a standard fallback placeholder string token inside the text field array
    strcpy(dialogState->inputTextBoxBuffer, "untitled");
    dialogState->inputTextLength = (int)strlen(dialogState->inputTextBoxBuffer);
}

void PR_UpdateFileDialog(MouseState mousePos) {
    FileDialogState *dialogState = PR_GetFileDialogState();
    if (!dialogState->isOpen) return;

    // 1. REFRESH CONTROL INTERACTION BUTTONS METRICS ROW
    PR_UpdateButton(&btnDialogCancel, mousePos);
    PR_UpdateButton(&btnDialogOK,     mousePos);

    if (btnDialogCancel.isPressed) {
        dialogState->isOpen = false;
        return;
    }

    // 2. PROCESS SUBMIT ACTION TRIGGER CONFIRMATION ROW
    if (btnDialogOK.isPressed) {
        if (dialogState->mode == FILE_DIALOG_OPEN) {
            if (dialogState->selectedIndex >= 0 && dialogState->selectedIndex < dialogState->fileCount) {
                dialogState->isOpen = false;
                snprintf(staticPathBuffer, sizeof(staticPathBuffer), "carts/%s", dialogState->files[dialogState->selectedIndex]);
                dialogState->onFileAction(staticPathBuffer);
                return;
            }
        } 
        else if (dialogState->mode == FILE_DIALOG_SAVE) {
            if (dialogState->inputTextLength > 0) {
                dialogState->isOpen = false;
                
                if (strstr(dialogState->inputTextBoxBuffer, dialogState->extensionFilter) == NULL) {
                    snprintf(staticPathBuffer, sizeof(staticPathBuffer), "carts/%s%s", 
                             dialogState->inputTextBoxBuffer, dialogState->extensionFilter);
                } else {
                    snprintf(staticPathBuffer, sizeof(staticPathBuffer), "carts/%s", dialogState->inputTextBoxBuffer);
                }
                
                dialogState->onFileAction(staticPathBuffer);
                return;
            }
        }
    }

    // FIX: Dynamically adapt display thresholds matching active viewport specifications
    int maxDisplayRows = (dialogState->mode == FILE_DIALOG_SAVE) ? 6 : 8;
    int maxClickableY  = (dialogState->mode == FILE_DIALOG_SAVE) ? 78 : 96; // 24 + (rows * 9) clamped cleanly

    // 3. HANDLE MOUSE INTERACTION AND SCROLL VIEWPORT OVER THE LIST BOX BOUNDS
    float wheel = GetMouseWheelMove();
    
    // FIX: Clamping scroll engine boundaries dynamically using active maxDisplayRows layout limits!
    // Prevents pushing empty blank ghost rows onto the bottom container field when scrolling targets
    if (wheel > 0 && dialogState->scrollOffset > 0) {
        dialogState->scrollOffset--;
    }
    if (wheel < 0 && dialogState->scrollOffset + maxDisplayRows < dialogState->fileCount) {
        dialogState->scrollOffset++;
    }

    // FIX: Expand mouse Y collision grid check to perfectly match maxClickableY (96px inside OPEN mode)!
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.x >= 12 && mousePos.x <= 116 && mousePos.y >= 24 && mousePos.y <= maxClickableY) {
        int clickedLine = (mousePos.y - 24) / 9;
        int targetedIndex = dialogState->scrollOffset + clickedLine;
        
        if (targetedIndex >= 0 && targetedIndex < dialogState->fileCount) {
            dialogState->selectedIndex = targetedIndex;
            
            if (dialogState->mode == FILE_DIALOG_SAVE) {
                char cleanTemp[64];
                strncpy(cleanTemp, dialogState->files[targetedIndex], 63);
                char *dot = strrchr(cleanTemp, '.');
                if (dot != NULL) *dot = '\0';
                
                strncpy(dialogState->inputTextBoxBuffer, cleanTemp, 31);
                dialogState->inputTextLength = (int)strlen(dialogState->inputTextBoxBuffer);
            }
        }
    }

    // 4. ADVANCED KEYBOARD CHARACTER INJECTOR SHIELD ENGINE (Strictly for SAVE Mode)
    if (dialogState->mode == FILE_DIALOG_SAVE) {
        int key = GetCharPressed();
        
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (dialogState->inputTextLength < 31)) {
                dialogState->inputTextBoxBuffer[dialogState->inputTextLength] = (char)key;
                dialogState->inputTextLength++;
                dialogState->inputTextBoxBuffer[dialogState->inputTextLength] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (dialogState->inputTextLength > 0) {
                dialogState->inputTextLength--;
                dialogState->inputTextBoxBuffer[dialogState->inputTextLength] = '\0';
            }
        }
    }
}

void PR_DrawFileDialog(void) {
    GraphicsSystem *gfx = PR_GetGraphicsSystem();
    FileDialogState *dialogState = PR_GetFileDialogState();
    if (!dialogState->isOpen) return;

    // Fetch dynamic systemic styling tokens parameters from theme capsule link safely
    ThemeRegistry theme = themeData[PR_GetActiveThemeId()];

    // 1. RENDER MASTER TRANSLUCENT OVERLAY FILTER SCREEN
    PR_RectFill(0, 0, 128, 128, P8_EXT_BLACK);

    // 2. RENDER MASTER FRAME BOARD EDGE CONSTRAINTS (Centred 112x106 layout box)
    PR_RectFill(8, 12, 112, 106, P8_LIGHT_GREY);
    PR_Rect(8, 12, 112, 106, P8_DARK_GREY);
    
    PR_RectFill(9, 13, 110, 9, P8_DARK_BLUE);
    
    const char *headerTitle = (dialogState->mode == FILE_DIALOG_SAVE) ? "SAVE CARTRIDGE FILE" : "OPEN CARTRIDGE FILE";
    PR_PrintPro(gfx->systemFontId, headerTitle, 14, 14, P8_WHITE);

    // 3. RENDER FILE LIST SCROLL VIEWPORT PANEL GATES
    // Height compressed to 56px in SAVE mode to make vertical space room for the input text field row!
    int listHeight = (dialogState->mode == FILE_DIALOG_SAVE) ? 56 : 74;
    PR_RectFill(12, 24, 104, listHeight, P8_BLACK);
    PR_Rect(12, 24, 104, listHeight, P8_DARK_GREY);

    int maxDisplayRows = (dialogState->mode == FILE_DIALOG_SAVE) ? 6 : 8;
    for (int i = 0; i < maxDisplayRows; i++) {
        int fileIndex = dialogState->scrollOffset + i;
        if (fileIndex >= dialogState->fileCount) break;

        int rowY = 26 + (i * 9);
        
        if (fileIndex == dialogState->selectedIndex) {
            PR_RectFill(14, rowY - 1, 100, 8, P8_BLUE);
            PR_PrintPro(gfx->systemFontId, dialogState->files[fileIndex], 16, rowY, P8_WHITE);
        } else {
            PR_PrintPro(gfx->systemFontId, dialogState->files[fileIndex], 16, rowY, P8_LIGHT_GREY);
        }
    }

    // 4. RENDER INTERACTIVE TEXT INPUT BOX PANEL LAYER (SAVE Mode Exclusive)
    if (dialogState->mode == FILE_DIALOG_SAVE) {
        PR_PrintPro(gfx->systemFontId, "FILENAME:", 12, 85, P8_DARK_GREY);
        
        // Input container shield frame border slot
        PR_RectFill(12, 92, 104, 9, P8_WHITE);
        PR_Rect(12, 92, 104, 9, P8_DARK_GREY);
        
        PR_PrintPro(gfx->systemFontId, dialogState->inputTextBoxBuffer, 16, 93, P8_BLACK);
        
        // Micro Blinking Caret text line feedback anchor
        // This ensures the cursor dynamically shifts exactly matching PR_Print's row width layout.
        if ((int)(GetTime() * 2) % 2 == 0) {
            int charWidth = fontData[PR_GetActiveFontId()].printableWidth;
            
            // Equation accounts for both character blocks and the implicit padding spaces between them
            int textW = dialogState->inputTextLength * (charWidth + 1);
            
            // Render the red pulse tracker bar safely aligned
            PR_RectFill(16 + textW, 93, 2, 7, P8_RED);
        }
    }

    // 5. BLIT ACTIVE BUTTONS COMMAND NODES
    PR_DrawButton(&btnDialogCancel);
    PR_DrawButton(&btnDialogOK);
}
