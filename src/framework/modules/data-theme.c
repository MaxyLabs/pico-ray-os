#include "../framework.h"

// --- CENTRAL SYSTEM UI THEME REGISTRY ---
// Completely independent from the hardware palette arrays
const ThemeRegistry themeData[THEME_COUNT] = {
    [THEME_LIGHT] = {
        .themeName       = "Light",
        .topBarBg        = P8_LIGHT_GREY,   // 6
        .topBarText      = P8_BLACK,        // 0
        .topBarHoverBg   = P8_EXT_SAND,     // 6
        .topBarHoverText = P8_BLACK,        // 0
        .menuCardBorder  = P8_BLACK,        // 0
        .menuCardBg      = P8_WHITE,        // 7
        .menuCardText    = P8_BLACK,        // 0
        .menuWindowBg    = P8_WHITE,        // 7
        .menuWindowText  = P8_BLACK,        // 0
        .menuWindowHoverBg   = P8_BLUE,     // 12
        .menuWindowHoverText = P8_WHITE,    // 7
        .buttonBg        = P8_DARK_GREY,    // 5
        .buttonHover     = P8_LIGHT_GREY,   // 6
        .buttonPressed   = P8_BLACK,        // 0
        .buttonIcon      = P8_WHITE,        // 7
        .buttonText      = P8_WHITE,        // 7
        .windowBg        = P8_LIGHT_GREY,   // 6
        .windowBorder    = P8_BLACK         // 0
    },
    [THEME_DARK] = {
        .themeName       = "Dark",
        .topBarBg        = P8_DARK_BLUE,    // 1 (Vagány sötétkék felső sáv)
        .topBarText      = P8_LIGHT_GREY,   // 6
        .topBarHoverBg   = P8_BLUE,         // 12
        .topBarHoverText = P8_WHITE,        // 7
        .menuCardBorder  = P8_PINK,         // 14 (Neon lila/rózsaszín keret)
        .menuCardBg      = P8_DARK_PURPLE,  // 2 (Sötétlila legördülő kártyák)
        .menuCardText    = P8_WHITE,        // 7
        .menuWindowBg    = P8_WHITE,        // 7
        .menuWindowText  = P8_BLACK,        // 0
        .menuWindowHoverBg   = P8_BLUE,     // 12
        .menuWindowHoverText = P8_WHITE,    // 7
        .buttonBg        = P8_DARK_PURPLE,  // 2
        .buttonHover     = P8_LAVENDER,     // 13
        .buttonPressed   = P8_BLACK,
        .buttonIcon      = P8_WHITE,
        .buttonText      = P8_WHITE,        // 7
        .windowBg        = P8_DARK_BLUE,    // 1
        .windowBorder    = P8_PINK          // 14
    },
    [THEME_HACKER_GREEN] = {
        .themeName       = "Terminal Green",
        .topBarBg        = P8_BLACK,        // 0
        .topBarText      = P8_GREEN,        // 11 (Zöld szöveg fekete alapon)
        .topBarHoverBg   = P8_BLUE,         // 12
        .topBarHoverText = P8_WHITE,        // 7
        .menuCardBg      = P8_BLACK,        // 0
        .menuCardBorder  = P8_DARK_GREEN,   // 3
        .menuCardText    = P8_GREEN,        // 11
        .menuWindowBg    = P8_WHITE,        // 7
        .menuWindowText  = P8_BLACK,        // 0
        .menuWindowHoverBg   = P8_BLUE,     // 12
        .menuWindowHoverText = P8_WHITE,    // 7
        .buttonBg        = P8_BLACK,        
        .buttonHover     = P8_DARK_GREEN,   // 3
        .buttonPressed   = P8_GREEN,        
        .buttonIcon      = P8_GREEN,        // 11
        .buttonText      = P8_GREEN,        // 11
        .windowBg        = P8_BLACK,        
        .windowBorder    = P8_GREEN         // 11
    }
};
