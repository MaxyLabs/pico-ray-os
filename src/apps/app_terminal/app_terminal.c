#include <string.h>
#include "../../framework/framework.h"

static ShellState shell;
static char terminalHomePath[MAX_CARTRIDGE_PATH_LEN] = {0};

// Forward function declaration
static void Command_cd(void);
static void Command_cls(void);
static void Command_help(void);
static void Command_load(void);
static void Command_load_git(void);
static void Command_ls(void);
static void Command_pwd(void);
static void Terminal_PushHistory(const char *text);

static void Command_cd(void) {
    // Check if it's exactly "cd" (Fallback straight to the virtual HOME location)
    if (strlen(shell.inputBuffer) == 2) {
        if (terminalHomePath[0] != '\0') {
            bool success = ChangeDirectory(terminalHomePath);
            if (success) {
                Terminal_PushHistory("Returned to HOME directory.");
                shell.lastReturnCode = 0;
            } else {
                Terminal_PushHistory("ERROR: Failed to recover HOME destination.");
                shell.lastReturnCode = 1;
            }
        } else {
            Terminal_PushHistory("ERROR: HOME path registry string is empty.");
            shell.lastReturnCode = 1;
        }
    } 
    // Check if it's "cd " followed by a target route parameter sequence
    else if (shell.inputBuffer[2] == ' ') {
        const char *targetPath = shell.inputBuffer + 3;
        
        bool success = ChangeDirectory(targetPath);
        
        if (success) {
            char successMsg[MAX_TERMINAL_ROW_LEN];
            PR_StrFormat(successMsg, sizeof(successMsg), "Moved to: %s", targetPath);
            Terminal_PushHistory(successMsg);
            shell.lastReturnCode = 0;
        } else {
            char errorMsg[MAX_TERMINAL_ROW_LEN];
            PR_StrFormat(errorMsg, sizeof(errorMsg), "ERROR: Cannot reach path '%s'", targetPath);
            Terminal_PushHistory(errorMsg);
            shell.lastReturnCode = 1;
            
            // Dynamic Powerline feedback hook: 
            // If it fails, our new badge will instantly render 'RC:1'!
        }
    }
}

static void Command_cls(void) {
    // Wipe out the scrollback buffer completely
    memset(shell.history, 0, sizeof(shell.history));
    shell.historyCount = 0;
}

static void Command_help(void) {
    Terminal_PushHistory("AVAILABLE COMMANDS:");
    Terminal_PushHistory("  help, cls, ls, pwd");
    Terminal_PushHistory("  load <file>, load_git <url>");

    // Terminal_PushHistory("  help              - Show this menu");
    // Terminal_PushHistory("  load <file>       - Load a local cartridge");
    // Terminal_PushHistory("  load_git <url>    - Load a cartridge from Git");
}

static void Command_load(void) {
    const char *filename = shell.inputBuffer + 5;
    printf("PICO-RAY OS | TERMINAL | Executing local load for: %s\n", filename);

    // Retrieve the global cartridge RAM pointer from the framework runtime
    CartridgeRAM *activeCart = PR_GetCartridgeRAM();
    
    if (activeCart != NULL) {
        bool success = PR_LoadRayCartridge(filename, activeCart);
        if (success) {
            Terminal_PushHistory("SYSTEM: Cartridge loaded successfully!");
            printf("PICO-RAY OS | TERMINAL | SYSTEM: Cartridge loaded successfully!\n");
        } else {
            char errorMsg[MAX_TERMINAL_ROW_LEN];
            PR_StrFormat(errorMsg, sizeof(errorMsg), "ERROR: Failed to load '%s'", filename);
            Terminal_PushHistory(errorMsg);
            shell.lastReturnCode = 1; // Error trigger
            printf("PICO-RAY OS | TERMINAL | ERROR: Failed to load '%s'", filename);
        }
    } else {
        Terminal_PushHistory("ERROR: Cartridge RAM system unavailable!");
        shell.lastReturnCode = 1;
        printf("PICO-RAY OS | TERMINAL | ERROR: System Cartridge RAM reference is unavailable!\n");
    }
}

static void Command_load_git(void) {
    const char *gitPath = shell.inputBuffer + 9;
    printf("PICO-RAY | TERMINAL | Parsing Git path parameters: %s\n", gitPath);
    
    // Static buffers to hold the extracted path component and branch tag name safely
    char cleanPath[MAX_CARTRIDGE_PATH_LEN] = {0};
    char branchName[32] = "main"; // Default fallback branch configuration
    
    // Look for the optional '@' branch separator modifier token inside the parameter string
    const char *atSymbol = strchr(gitPath, '@');
    
    if (atSymbol != NULL) {
        // Extract the branch name sequence string safely from behind the '@' symbol
        PR_StrlCpy(branchName, atSymbol + 1, sizeof(branchName));
        
        // Compute the exact length of the preceding clean file path segment matrix
        size_t pathLen = atSymbol - gitPath;
        if (pathLen < sizeof(cleanPath)) {
            PR_StrlCpy(cleanPath, gitPath, pathLen + 1);
            cleanPath[pathLen] = '\0'; // Enforce secure closure pass
        } else {
            PR_StrlCpy(cleanPath, gitPath, sizeof(cleanPath));
        }
    } else {
        // No branch token found, the whole parameter represents the target file location
        PR_StrlCpy(cleanPath, gitPath, sizeof(cleanPath));
    }
    
    // Buffer to assemble the final raw.githubusercontent web string target dynamically
    char constructedUrl[512];
    
    // Inject both the clean path profile and the active branch name token into the URL template
    bool urlOk = PR_StrFormat(constructedUrl, sizeof(constructedUrl), 
                              "https://githubusercontent.com", 
                              /* Owner & Repo split tracking is handled by layout */
                              /* We need to insert branch right between repo and file path! */
                              /* Let's parse the string to isolate the owner/repo from the file path */
                              ""); // Placeholder for dynamic structural string tracking
    
    // RE-ARCHITECTURE HINT: To properly place the branch tag, we need to locate the second slash '/'
    // Let's refine the split logic to map <owner>/<repo>/<file_path> properly.
    const char *firstSlash = strchr(cleanPath, '/');
    if (firstSlash != NULL) {
        const char *secondSlash = strchr(firstSlash + 1, '/');
        if (secondSlash != NULL) {
            char ownerRepo[128] = {0};
            size_t orLen = secondSlash - cleanPath;
            if (orLen < sizeof(ownerRepo)) {
                PR_StrlCpy(ownerRepo, cleanPath, orLen + 1);
            }
            
            const char *filePath = secondSlash + 1;
            
            // Final seamless URL compilation pass utilizing our safe formatter engine
            urlOk = PR_StrFormat(constructedUrl, sizeof(constructedUrl),
                                 "https://raw.githubusercontent.com/%s/refs/heads/%s/%s",
                                 ownerRepo, branchName, filePath);
        } else {
            urlOk = false;
        }
    } else {
        urlOk = false;
    }
    
    if (!urlOk) {
        Terminal_PushHistory("ERROR: Malformed GitHub parameter format profile.");
        shell.lastReturnCode = 127;
    } else {
        printf("PICO-RAY | TERMINAL | Target verified: %s\n", constructedUrl);
        Terminal_PushHistory("SYSTEM: Connecting to GitHub...");
        
        const char *tempDownloadPath = "carts/temp_git.ray";
        bool downloadSuccess = PR_Network_DownloadFile(constructedUrl, tempDownloadPath);
        
        if (downloadSuccess) {
            Terminal_PushHistory("SYSTEM: Download finished. Parsing cartridge...");
            
            CartridgeRAM *activeCart = PR_GetCartridgeRAM();
            if (activeCart != NULL) {
                bool parseSuccess = PR_LoadRayCartridge(tempDownloadPath, activeCart);
                
                if (parseSuccess) {
                    Terminal_PushHistory("SYSTEM: Git cartridge successfully loaded!");
                    shell.lastReturnCode = 0;
                } else {
                    Terminal_PushHistory("ERROR: Failed to parse downloaded .ray file.");
                    shell.lastReturnCode = 2;
                }
            } else {
                Terminal_PushHistory("ERROR: Cartridge RAM system unavailable.");
                shell.lastReturnCode = 1;
            }
        } else {
            Terminal_PushHistory("ERROR: Network transfer failed via libcurl.");
            shell.lastReturnCode = 1;
        }
    }
}

static void Command_ls(void) {
    // Utilizing portable raylib directory extraction loop matrix
    FilePathList files = LoadDirectoryFiles(".");
    Terminal_PushHistory("DIRECTORY FILES:");
    
    int showCount = (files.count < 8) ? files.count : 8; // Safeguard terminal screen limits
    for (unsigned int i = 0; i < (unsigned int)showCount; i++) {
        char fileMsg[MAX_TERMINAL_ROW_LEN];
        PR_StrFormat(fileMsg, sizeof(fileMsg), "  %s", files.paths[i]);
        Terminal_PushHistory(fileMsg);
    }
    
    if (files.count > (unsigned int)showCount) {
        Terminal_PushHistory("  ... [truncated]");
    }
    
    UnloadDirectoryFiles(files); // Free raylib allocated memory safety pass
}

static void Command_pwd(void) {
    // Utilizing portable raylib utility to query active context directory path
    const char *currentDir = GetWorkingDirectory();
    if (currentDir != NULL) {
        char dirMsg[MAX_TERMINAL_ROW_LEN];
        PR_StrFormat(dirMsg, sizeof(dirMsg), "%s", currentDir);
        Terminal_PushHistory(dirMsg);
    } else {
        Terminal_PushHistory("ERROR: Could not resolve working directory.");
        shell.lastReturnCode = 1;
    }
}

// Safely pushes a new text line string into the terminal history list matrix
static void Terminal_PushHistory(const char *text) {
    if (shell.historyCount < MAX_TERMINAL_ROWS) {
        // Space is still available, copy directly into the next open slot
        PR_StrlCpy(shell.history[shell.historyCount], text, MAX_TERMINAL_ROW_LEN - 1);
        shell.historyCount++;
    } else {
        // Buffer is full, shift all previous rows up by 1 slot to clear the bottom
        for (int i = 1; i < MAX_TERMINAL_ROWS; i++) {
            strcpy(shell.history[i - 1], shell.history[i]);
        }
        // Insert the fresh line string array at the newly cleared bottom slot
        PR_StrlCpy(shell.history[MAX_TERMINAL_ROWS - 1], text, MAX_TERMINAL_ROW_LEN - 1);
    }
}

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
    shell.promptY = 16.0f; // Sits safely 4 pixels below the 10px tall MenuBar!
    shell.isCursorVisible = true;

    // Capture the initial portable startup directory to act as the virtual HOME route
    const char *homeDir = GetWorkingDirectory();
    if (homeDir != NULL) {
        PR_StrlCpy(terminalHomePath, homeDir, sizeof(terminalHomePath));
    }
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
        // printf("PICO-RAY OS | DEBUG | char: %c, keyChar: %d, printable: %d\n", (char)keyChar, keyChar, (int)charPrintable);

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
            printf("PICO-RAY OS | TERMINAL | SUBMITTED COMMAND: %s\n", shell.inputBuffer);

            // Log the user's input command into the history scrollback buffer
            char echoBuffer[MAX_TERMINAL_ROW_LEN];
            PR_StrFormat(echoBuffer, sizeof(echoBuffer), "> %s", shell.inputBuffer);
            Terminal_PushHistory(echoBuffer);

            // Default return code is 0 (Success) until a gate overrides it
            shell.lastReturnCode = 0;

            // --- COMMAND: CD ---
            if (strncmp(shell.inputBuffer, "cd", 2) == 0) {
                Command_cd();
            }
            // --- COMMAND: CLS ---
            else if (strcmp(shell.inputBuffer, "cls") == 0) {
                Command_cls();
            }
            // --- COMMAND: HELP ---
            else if (strcmp(shell.inputBuffer, "help") == 0) {
                Command_help();
            }
            // --- COMMAND: LOAD ---
            else if (strncmp(shell.inputBuffer, "load ", 5) == 0) {
                Command_load();
            }
            // --- COMMAND: LOAD_GIT ---
            else if (strncmp(shell.inputBuffer, "load_git ", 9) == 0) {
                Command_load_git();
            }
            // --- COMMAND: LS ---
            else if (strcmp(shell.inputBuffer, "ls") == 0) {
                Command_ls();
            }
            // --- COMMAND: PWD ---
            else if (strcmp(shell.inputBuffer, "pwd") == 0) {
                Command_pwd();
            }
            
            // --- UNKNOWN COMMAND ---
            else {
                char unknownMsg[MAX_TERMINAL_ROW_LEN];
                if (!PR_StrFormat(unknownMsg, sizeof(unknownMsg), "Unknown command: %s", shell.inputBuffer)) {
                    PR_PushKernelWarning("Terminal overflow: Unknown command string truncated.");
                }
                Terminal_PushHistory(unknownMsg);
                shell.lastReturnCode = 127; // Standard UNIX unknown command code
                printf("PICO-RAY OS | TERMINAL | Unknown command sequence: %s\n", shell.inputBuffer);
            }

            // Clean out the active line buffer row safely for the next input cycle
            memset(shell.inputBuffer, 0, sizeof(shell.inputBuffer));
            shell.cursorPosition = 0;
        }
    }
}

void Terminal_Draw(void) {
    // Clear the active terminal workspace screen canvas to a nostalgic deep console black
    PR_Cls(P8_BLACK);

    float currentY = 8.0f; // Sits safely below the MenuBar bounds
    //float lineSpacing = 10.0f;
    float lineSpacing = 8.0f;

    // 1. RENDER HISTORICAL SCROLLBACK BUFFER ROWS
    for (int i = 0; i < shell.historyCount; i++) {
        PR_PrintPro(FONT_PICORAY, shell.history[i], shell.promptX, currentY, P8_LIGHT_GREY);
        currentY += lineSpacing;
    }

    // 2. RENDER THE ACTIVE INTERACTIVE PROMPT LINE
    float promptWidthOffset = shell.promptX;    

    // Powerline dynamic gate: if the last command failed, inject a crimson error badge
    if (shell.lastReturnCode != 0) {
        char rcBuffer[16];
        
        if (shell.lastReturnCode == 1) {
            snprintf(rcBuffer, sizeof(rcBuffer), "ERROR ");
        } else if (shell.lastReturnCode == 127) {
            snprintf(rcBuffer, sizeof(rcBuffer), "NOTFOUND ");
        } else {
            snprintf(rcBuffer, sizeof(rcBuffer), "RC:%d ", shell.lastReturnCode);
        }

        // Measure pixel footprint to draw a clean background solid badge
        int badgeWidth = PR_GetStringWidthByPixel(FONT_PICORAY, rcBuffer);
        
        // Draw solid background block for the powerline segment
        PR_RectFill(promptWidthOffset, currentY - 2, (float)badgeWidth, 8.0f, P8_RED);
        // Print the return code inside the red badge using white text
        PR_PrintPro(FONT_PICORAY, rcBuffer, promptWidthOffset, currentY, P8_WHITE);
        
        promptWidthOffset += (float)badgeWidth + 4.0f; // Shift the input text rightwards
    }

    // Draw the main prompt arrow
    PR_PrintPro(FONT_PICORAY, "> ", promptWidthOffset, currentY, P8_GREEN);
    promptWidthOffset += 8.0f;

    // Render the active typed buffer string
    PR_PrintPro(FONT_PICORAY, shell.inputBuffer, promptWidthOffset, currentY, P8_WHITE);

    //PR_PrintPro(FONT_PICORAY, "> ", shell.promptX, currentY, P8_GREEN);
    //PR_PrintPro(FONT_PICORAY, shell.inputBuffer, shell.promptX + 8.0f, currentY, P8_WHITE);

    // 3. RENDER THE ANIMATED BLINKING CURSOR BLOCK
    if (shell.isCursorVisible) {
        int inputPixelWidth = PR_GetStringWidthByPixel(FONT_PICORAY, shell.inputBuffer);
        float cursorOffsetX = promptWidthOffset + inputPixelWidth; 

        PR_PrintPro(FONT_PICORAY, "_", cursorOffsetX, currentY, P8_GREEN);
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
