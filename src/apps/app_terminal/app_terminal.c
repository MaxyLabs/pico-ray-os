#include <string.h>
#include "../../framework/framework.h"

static ShellState shell;

AppConfig Terminal_GetConfig(void) {
    AppConfig config = {
        .name       = "Terminal",
        .author     = "Gergely Macoun,\nAI_Collaborator",
        .version    = "0.1.0",
        .license    = "MIT",
        .iconId     = ICON_TERMINAL,
        .appId      = (unsigned int)SYS_APP_TERMINAL,
        .width      = 256,
        .height     = 128,
        .hasMenuBar = true
    };
    return config;
}

void Terminal_RegisterMenus(void) {
}

void Terminal_Init(void) {
    memset(&shell, 0, sizeof(ShellState));
    shell.promptX = 4.0f;
    shell.promptY = 14.0f; // Sits safely 4 pixels below the 10px tall MenuBar!
    shell.isCursorVisible = true;
}

void Terminal_Update(MouseState mousePos) {
    // 1. CURSOR FLASHING TICK TIMER ANIMATION
    shell.blinkCounter++;
    if (shell.blinkCounter >= 30) {
        // Toggle cursor visibility roughly every half second (30 frames)
        shell.isCursorVisible = !shell.isCursorVisible;
        shell.blinkCounter = 0;
    }

    // 2. HARDWARE KEYBOARD CHARACTER INPUT CAPTURE STREAM
    int keyChar = GetCharPressed();
    while (keyChar > 0) {
        bool charPrintable = PR_IsCharPrintable(FONT_PICORAY_UTF8, (unsigned int)keyChar);

        // DEBUG
        printf("PICO-RAY | DEBUG | char: %c, keyChar: %d, printable: %d\n", (char)keyChar, keyChar, (int)charPrintable);

        // FIX 1: Check printability against our master multi-lingual FONT_PR_UTF8 profile!
        if (charPrintable) {
            // FIX 2: REVOLUTIONARY MULTI-BYTE UTF-8 ENCODER
            // Instead of raw casting, we safely encode the 32-bit codepoint into proper UTF-8 bytes.
            int byteLength = 0;
            const char *utf8Bytes = CodepointToUTF8(keyChar, &byteLength);

            // Ensure the incoming multi-byte character physically fits into our buffer layout bounds
            if (shell.cursorPosition + byteLength < (MAX_SHELL_COMMAND_LEN - 1)) {
                // Stream the encoded byte sequence straight into the active input line buffer
                for (int i = 0; i < byteLength; i++) {
                    shell.inputBuffer[shell.cursorPosition++] = utf8Bytes[i];
                }
                shell.inputBuffer[shell.cursorPosition] = '\0'; // Always guarantee secure closure
                
                shell.isCursorVisible = true; 
                shell.blinkCounter = 0;
            }
        }
        keyChar = GetCharPressed(); // Query next character in the frame hardware buffer queue loop
    }

    // 3. BACKSPACE DELETION HANDLING GATE
    if (IsKeyPressed(KEY_BACKSPACE) && shell.cursorPosition > 0) {
        // INTELLIGENT UTF-8 MULTI-BYTE BACKSPACE ERASER LOOP
        // Because accented characters take 2 bytes in UTF-8, a simple single decrement would break the string!
        // We step back byte by byte until we hit the starting byte of the character (non-continuation byte).
        // In UTF-8, continuation bytes always start with bits 10xxxxxx (0x80 to 0xBF).
        do {
            shell.cursorPosition--;
        } while (shell.cursorPosition > 0 && 
                ((unsigned char)shell.inputBuffer[shell.cursorPosition] & 0xC0) == 0x80);

        shell.inputBuffer[shell.cursorPosition] = '\0'; // Erase the character slot cleanly
        shell.isCursorVisible = true;
        shell.blinkCounter = 0;
    }

    // 4. ENTER COMMAND SUBMIT GATE
    if (IsKeyPressed(KEY_ENTER)) {
        if (shell.cursorPosition > 0) {
            printf("PICO-RAY | TERMINAL | SUBMITTED COMMAND: %s\n", shell.inputBuffer);

            // Temporary command line clear-out pass until we build the Text Scrollback Buffer!
            memset(shell.inputBuffer, 0, sizeof(shell.inputBuffer));
            shell.cursorPosition = 0;
        }
    }
}

void Terminal_Draw(void) {
    // Clear the active terminal workspace screen canvas to a nostalgic deep console black
    PR_Cls(P8_BLACK);

    // Render the active Shell active Prompt input string 
    // We enforce the rigid system font profile 0 (FONT_PICORAY) to prevent game overlaps!
    PR_PrintPro(FONT_PICORAY, "> ", shell.promptX, shell.promptY, P8_GREEN);
    PR_PrintPro(FONT_PICORAY, shell.inputBuffer, shell.promptX + 8.0f, shell.promptY, P8_WHITE);

    // Render the animated blinking cursor block if toggled active
    if (shell.isCursorVisible) {
        // We query the exact pixel width footprint of whatever characters are currently inside the buffer!
        int inputPixelWidth = PR_GetStringWidthByPixel(FONT_PICORAY, shell.inputBuffer);

        // Cursor X position = Prompt starting point + "> " prompt text size + dynamic string width
        float cursorOffsetX = shell.promptX + 8.0f + inputPixelWidth; 

        PR_PrintPro(FONT_PICORAY, "_", cursorOffsetX, shell.promptY, P8_GREEN);
        //PR_PrintPro(FONT_PICORAY, "_", cursorOffsetX, liveRowY, P8_GREEN);
    }
}

void Terminal_Cleanup(void) {
}

AppInterface Terminal_App = {
    .GetConfig     = Terminal_GetConfig,
    .Init          = Terminal_Init,
    .RegisterMenus = Terminal_RegisterMenus,
    .Update        = Terminal_Update,
    .Draw          = Terminal_Draw,
    .Cleanup       = Terminal_Cleanup
};
