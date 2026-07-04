#include "../framework.h"

// --- CORE UTILITY: Safe Color Translation API ---

// --- Safely retrieve a real Raylib Color from the active system palette ---
Color PR_GetColor(int colorId) {
    int currentPaletteId = PR_GetActivePaletteId();
    
    // Read the maximum available color count from our unified palette registry
    int count = paletteData[currentPaletteId].colorCount;
    
    // Perform safe modulo wrapping and return the final structural Color object
    return paletteData[currentPaletteId].colors[colorId % count];
}

// Visszaadja a PICO-8 színkódot (0-15) egy valós Color struktúra alapján
// Ha a pixel átlátszó (transzparens), a PICO-8-as 0-s (fekete) kódot adja vissza
int GetPicoColorFromColor(Color c) {
    int currentPaletteId = PR_GetActivePaletteId();
    int count = paletteData[currentPaletteId].colorCount;

    if (c.a == 0) return 0; 

    // Scan directly through the embedded structural color matrix stack
    for (int i = 0; i < count; i++) {
        if (paletteData[currentPaletteId].colors[i].r == c.r && 
            paletteData[currentPaletteId].colors[i].g == c.g && 
            paletteData[currentPaletteId].colors[i].b == c.b) {
            return i; // Match established!
        }
    }

    return 0; // Biztonsági mentőöv, ha a szín nem része a palettának
}

// --- BELSŐ SÚGÓ FÜGGVÉNYEK AZ EXPORTÁLÁSHOZ ---
char GetHexCharFromColor(Color c) {
    int currentPaletteId = PR_GetActivePaletteId();
    int count = paletteData[currentPaletteId].colorCount;

    if (c.a == 0) return '.'; 

    for (int i = 0; i < count; i++) {
        if (paletteData[currentPaletteId].colors->r == c.r && 
            paletteData[currentPaletteId].colors->g == c.g && 
            paletteData[currentPaletteId].colors->b == c.b) {
            if (i < 10) return '0' + i;
            return 'A' + (i - 10);
        }
    }

    return '0'; 
}
