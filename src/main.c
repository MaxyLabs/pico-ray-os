#include "raylib.h"
#include "framework/framework.h"

int main(void) {
    // Fire up the central operating system kernel and open the initial screen
    Framework_Init();

    // MAIN OPERATING SYSTEM RUNTIME LOOP
    while (!WindowShouldClose() && !PR_GetShouldQuitOS()) {
        Framework_Update();
        Framework_Draw();
    }

    // Gracefully free system textures and shut down the hardware window viewport
    Framework_Cleanup();    
    CloseWindow();

    return 0;
}
