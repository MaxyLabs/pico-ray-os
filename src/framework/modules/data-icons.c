#include "../framework.h"

const IconRegistry iconData[ICON_COUNT] = {
    // Official OS Logo: Bouncing ray and top starburst
    [ICON_PICORAY] = {
        .iconName = "pico-ray",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            ".X.X.X",  // Starburst sparks at the top
            "..XXX.",
            "...XXX",  // Impact focal point surface
            "..X.X.",
            ".X...X",  // Ascending light beam coordinates
            "X....."   // The Ray foundation start
        }
    },
    // Tool / Brush
    // A neat, round bitmapped apple shape
    [ICON_APPLE] = {
        .iconName = "apple",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..XX..",
            ".XXXX.",
            "XXXXXX",
            "XXXXXX",
            ".XXXX.",
            "..XX.."
        },
    },
   // Tool / Brush
    [ICON_BRUSH] = {
        .iconName = "brush",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "....X.",
            "...XXX",
            "..XXX.",
            ".X.X..",
            ".XXX..",
            "..XX.."
        }
    },
    // close
    [ICON_CLOSE] = {
        .iconName = "close",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "......",
            ".X.X..",
            "..X...",
            ".X.X..",
            "......",
            "......"
        },
    },
    // Down Arrow
    [ICON_DOWN] = {
        .iconName = "down",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "......",
            "......",
            "XXXXX.",
            ".XXX..",
            "..X...",
            "......"
        }
    },
    // Directory folder tab layout
    [ICON_FOLDER] = {
        .iconName = "folder",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "XX....",
            "XXXXXX",
            "X....X",
            "XXXXXX",
            "XXXXXX",
            "XXXXXX"
        }
    },
   // Tool / Flood / Paint Bucket
    [ICON_FLOOD] = {
        .iconName = "flood",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "...X..",
            "..X.X.",
            ".X...X",
            "X.XXX.",
            "X..X..",
            "......"
        }
    },
    // App / Sprite Editor
    [ICON_FONT_STUDIO] = {
        .iconName = "font-studio",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "XXXXXX",
            "X.XX.X",
            "X.XX.X",
            "..XX..",
            "..XX..",
            ".XXXX."
        }
    },
    // Classic cottage house
    [ICON_HOME] = {
        .iconName = "home",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..XX..",
            ".XXXX.",
            "XXXXXX",
            "XX..XX",
            "XX..XX",
            "XXXXXX"
        }
    },
    // Info
    [ICON_INFO] = {
        .iconName = "info",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..XX..",
            "......",
            ".XXXX.",
            "..XX..",
            "..XX..",
            ".XXXX."
        },
    },
    // Left Arrow
    [ICON_LEFT] = {
        .iconName = "left",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "...X..",
            "..XX..",
            ".XXX..",
            "..XX..",
            "...X..",
            "......"
        }
    },
    // Lua
    [ICON_LUA] = {
        .iconName = "lua",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            ".....X",
            "..XX..",
            ".XX.X.",
            ".XXXX.",
            "..XX..",
            "......"
        }
    },
    // Pause
    [ICON_PAUSE] = {
        .iconName = "pause",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "......",
            "..X.X.",
            "..X.X.",
            "..X.X.",
            "......",
            "......"
        }
    },
    // PLAY
    [ICON_PLAY] = {
        .iconName = "play",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..X...",
            "..XX..",
            "..XXX.",
            "..XX..",
            "..X...",
            "......"
        }
    },
    // Print
    [ICON_PRINT_LEFT] = {
        .iconName = "print-left",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "XXXXXX",
            "...X.X",
            "XX.XXX",
            "...X.X",
            "XX.XXX",
            "...X.X"
        }
    },
    [ICON_PRINT_RIGHT] = {
        .iconName = "print-right",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "XXXXXX",
            "X.X...",
            "XXX.XX",
            "X.X...",
            "XXX.XX",
            "X.X..."
        }
    },
    // Pencil
    [ICON_PENCIL] = {
        .iconName = "pencil",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "....X.",
            "...XXX",
            "..XXX.",
            ".X.X..",
            ".XX...",
            "......"
        }
    },
    // Plugin
    [ICON_PLUGIN] = {
        .iconName = "plugin",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "X.X..X",
            "XXX..X",
            "XXX..X",
            ".X..X.",
            ".X..X.",
            "..XX.."
        }
    },
    // Quit
    [ICON_QUIT] = {
        .iconName = "quit",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            ".XXX..",
            "X.X.X.",
            "XX.XX.",
            "X.X.X.",
            ".XXX..",
            "......"
        },
    },
    // Reload
    [ICON_RELOAD] = {
        .iconName = "reload",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..XX..",
            "....X.",
            "XX..X.",
            ".X..X.",
            "..XX..",
            "......"
        },
    },
    // Right Arrow
    [ICON_RIGHT] = {
        .iconName = "right",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..X...",
            "..XX..",
            "..XXX.",
            "..XX..",
            "..X...",
            "....."
        }
    },
     // Save / Floppy Icon
    [ICON_SAVE] = {
        .iconName = "save",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "XXXXXXX",
            "X.XX.X",
            "X.XX.X",
            "X....X",
            "X....X",
            "XXXXXX"
        }
    },
    // App / Sprite Editor
    [ICON_SPRITE_EDITOR] = {
        .iconName = "sprite-editor",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "...X.X",
            "..X.X.",
            ".X.X.X",
            ".XX.X.",
            ".XXX..",
            "X....."
        }
    },
    // Stop
    [ICON_STOP] = {
        .iconName = "stop",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "......",
            "..XXX.",
            "..XXX.",
            "..XXX.",
            "......",
            "......"
        }
    },
     // App / Terminal
    [ICON_TERMINAL] = {
        .iconName = "terminal",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "X.....",
            "X.X...",
            "X..X..",
            "X.X...",
            "X...XX",
            "......"
        }
    },
    // Wrench / hammer diagonal blueprint silhouette
    [ICON_TOOLS] = {
        .iconName = "tools",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "..X...",
            "X.X...",
            "XXXX..",
            "...XX.",
            "....XX",
            ".....X"
        }
    },
    // Up Arrow
    [ICON_UP] = {
        .iconName = "up",
        .iconWidth = 6,
        .iconHeight = 6,
        .data = {
            "......",
            "..X...",
            ".XXX..",
            "XXXXX.",
            "......",
            "......"
        }
    }
};
