#include "../framework.h"

MouseState PR_GetMousePosition(void) {
    GraphicsSystem *graphicsSystem = PR_GetGraphicsSystem();
    MouseState mousePos;

    mousePos.x = (int)(((float)GetMouseX() / (float)GetScreenWidth())  * graphicsSystem->virtualWidth  + 0.5f);
    mousePos.y = (int)(((float)GetMouseY() / (float)GetScreenHeight()) * graphicsSystem->virtualHeight + 0.5f);

    // Boundary safety checking: never allow coordinates to overflow virtual bounds
    if (mousePos.x < 0) mousePos.x = 0;
    if (mousePos.x >= graphicsSystem->virtualWidth)  mousePos.x = graphicsSystem->virtualWidth - 1;
    if (mousePos.y < 0) mousePos.y = 0;
    if (mousePos.y >= graphicsSystem->virtualHeight) mousePos.y = graphicsSystem->virtualHeight - 1;

    return mousePos;
}
