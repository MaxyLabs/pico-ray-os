#include "../framework.h"

// --- CENTRAL SYSTEM PALETTE DATA REGISTRY ---
const PaletteRegistry paletteData[PALETTE_COUNT] = {
    // --- OFFICIAL PICO-8 PALETTE (32 Colors) ---
    // Note:
    // The PICO-8 16-color palette sequence was originally curated by Lexaloffle Games.
    [PALETTE_PICO8] = {
        .paletteName = "PICO-8",
        .colorCount = 32,
        .colors = {
            { 0x00, 0x00, 0x00, 255 },  // #000000  Black
            { 0x1D, 0x2B, 0x53, 255 },  // #1D2B53  Dark Blue 
            { 0x7E, 0x25, 0x53, 255 },  // #7E2553  Dark Purple 
            { 0x00, 0x87, 0x51, 255 },  // #008751  Dark Green 
            { 0xAB, 0x52, 0x36, 255 },  // #AB5236  Brown
            { 0x5F, 0x57, 0x4F, 255 },  // #5F574F  Dark Grey 
            { 0xC2, 0xC3, 0xC7, 255 },  // #C2C3C7  Light Grey 
            { 0xFF, 0xF1, 0xE8, 255 },  // #FFF1E8  White
            { 0xFF, 0x00, 0x4D, 255 },  // #FF004D  Red
            { 0xFF, 0xA3, 0x00, 255 },  // #FFA300  Orange
            { 0xFF, 0xEC, 0x27, 255 },  // #FFEC27  Yellow
            { 0x00, 0xE4, 0x36, 255 },  // #00E436  Green
            { 0x29, 0xAD, 0xFF, 255 },  // #29ADFF  Blue
            { 0x83, 0x76, 0x9C, 255 },  // #83769C  Lavender
            { 0xFF, 0x77, 0xA8, 255 },  // #FF77A8  Pink
            { 0xFF, 0xCC, 0xAA, 255 },  // #FFCCAA  Light Peach 

            { 0x29, 0x18, 0x14, 255 },  // #291814  Brownish Black
            { 0x11, 0x1D, 0x35, 255 },  // #111D35  Darker Blue
            { 0x42, 0x21, 0x36, 255 },  // #422136  Darker Purple
            { 0x12, 0x53, 0x59, 255 },  // #125359  Blue Green
            { 0x74, 0x2F, 0x29, 255 },  // #742F29  Dark Brown
            { 0x49, 0x33, 0x3B, 255 },  // #49333B  Darker Grey
            { 0xA2, 0x88, 0x79, 255 },  // #A28879  Medium Grey
            { 0xF3, 0xEF, 0x7D, 255 },  // #F3EF7D  Light Yellow
            { 0xBE, 0x12, 0x50, 255 },  // #BE1250  Dark Red
            { 0xFF, 0x6C, 0x24, 255 },  // #FF6C24  Dark Orange
            { 0xA8, 0xE7, 0x2E, 255 },  // #A8E72E  Lime Green
            { 0x00, 0xB5, 0x43, 255 },  // #00B543  Medium Green
            { 0x06, 0x5A, 0xB5, 255 },  // #065AB5  True Blue
            { 0x75, 0x46, 0x65, 255 },  // #754665  Mauve
            { 0xFF, 0x6E, 0x59, 255 },  // #FF6E59  Dark Peach
            { 0xFF, 0x9D, 0x81, 255 }   // #FF9D81  Peach
        }
    },
    // --- PYXEL PALETTE (32 Colors) ---
    [PALETTE_PYXEL] = {
        .paletteName = "Pyxel",
        .colorCount = 32,
        .colors = {
            { 0x00, 0x00, 0x00, 255 },  // #000000  Black
            { 0x2B, 0x33, 0x5F, 255 },  // #2b335f  Dark Blue
            { 0x7E, 0x20, 0x72, 255 },  // #7e2072  Dark Purple
            { 0x19, 0x95, 0x9C, 255 },  // #19959c  Dark Green
            { 0x8B, 0x48, 0x52, 255 },  // #8b4852  Brown
            { 0x39, 0x5C, 0x98, 255 },  // #395c98  Slate Gray
            { 0xA9, 0xC1, 0xFF, 255 },  // #a9c1ff  Light Blue
            { 0xEE, 0xEE, 0xEE, 255 },  // #eeeeee  White
            { 0xD4, 0x18, 0x6C, 255 },  // #d4186c  Red
            { 0xD3, 0x84, 0x41, 255 },  // #d38441  Orange
            { 0xE9, 0xC3, 0x5B, 255 },  // #e9c35b  Yellow
            { 0xA3, 0xA3, 0xA3, 255 },  // #a3a3a3  Green
            { 0x70, 0xC6, 0xA9, 255 },  // #70c6a9  Mint Green
            { 0x76, 0x96, 0xDE, 255 },  // #7696de  Sky Blue
            { 0xFF, 0x97, 0x98, 255 },  // #ff9798  Pink
            { 0xED, 0xC7, 0xB0, 255 },  // #edc7b0  Peach

            { 0x07, 0x06, 0x0c, 255 },  // #07060c  Deep Obsidian
            { 0x2B, 0x33, 0x5F, 255 },  // #121530  Abyssal Indigo
            { 0x7E, 0x20, 0x72, 255 },  // #460d41  Shadow Plum
            { 0x19, 0x95, 0x9C, 255 },  // #0a5459  Deep Teal / Pine
            { 0x8B, 0x48, 0x52, 255 },  // #522129  Burnt Bark
            { 0x39, 0x5C, 0x98, 255 },  // #1b3054  Steel Navy
            { 0xA9, 0xC1, 0xFF, 255 },  // #607cb3  Periwinkle
            { 0xEE, 0xEE, 0xEE, 255 },  // #b3b3b3  Ash Gray
            { 0xD4, 0x18, 0x6C, 255 },  // #80083c  Crimson Maroon
            { 0xD3, 0x84, 0x41, 255 },  // #874c1a  Rust / Terracotta
            { 0xE9, 0xC3, 0x5B, 255 },  // #9a7c29  Ochre Gold
            { 0xA3, 0xA3, 0xA3, 255 },  // #616161  Muddy Olive
            { 0x70, 0xC6, 0xA9, 255 },  // #3b846e  Seafoam Shadow
            { 0x76, 0x96, 0xDE, 255 },  // #3e5ca1  Deep Ocean
            { 0xFF, 0x97, 0x98, 255 },  // #b85759  Muted Coral
            { 0xED, 0xC7, 0xB0, 255 }   // #ab836b  Warm Tan
        }
    },
    // --- COMMODORE 64 HARDWARE PALETTE (16 Colors) ---
    // https://lospec.com/palette-list/commodore64
    // https://lospec.com/palette-list/saturatedc64
    [PALETTE_C64] = {
        .paletteName = "C64",
        .colorCount = 32,
        .colors = {
            { 0x00, 0x00, 0x00, 255 },  // #000000  Black
            { 0x50, 0x45, 0x9b, 255 },  // #50459b  Blue
            { 0xa0, 0x57, 0xa3, 255 },  // #a057a3  Purple
            { 0x5c, 0xab, 0x5e, 255 },  // #5cab5e  Green
            { 0x6d, 0x54, 0x12, 255 },  // #6d5412  Brown
            { 0x62, 0x62, 0x62, 255 },  // #626262  Dark Grey
            { 0x89, 0x89, 0x89, 255 },  // #898989  Grey
            { 0xff, 0xff, 0xff, 255 },  // #ffffff  White
            { 0x9f, 0x4e, 0x44, 255 },  // #9f4e44  Red
            { 0xa1, 0x68, 0x3c, 255 },  // #a1683c  Orange
            { 0xc9, 0xd4, 0x87, 255 },  // #c9d487  Yellow
            { 0x9a, 0xe2, 0x9b, 255 },  // #9ae29b  Light Green
            { 0x6a, 0xbf, 0xc6, 255 },  // #6abfc6  Cyan
            { 0x88, 0x7e, 0xcb, 255 },  // #887ecb  Light Blue
            { 0xcb, 0x7e, 0x75, 255 },  // #cb7e75  Light Red
            { 0xad, 0xad, 0xad, 255 },  // #adadad  Light Grey
  
            { 0x00, 0x00, 0x00, 255 },  // #000000  Saturated Black
            { 0x45, 0x2e, 0xe6, 255 },  // #452ee6  Saturated Blue
            { 0xae, 0x1f, 0xdb, 255 },  // #ae1fdb  Saturated Purple
            { 0x1f, 0xa3, 0x22, 255 },  // #1fa322  Saturated Green
            { 0x99, 0x43, 0x12, 255 },  // #994312  Saturated Brown
            { 0x36, 0x3a, 0x4a, 255 },  // #363a4a  Saturated Dark Grey
            { 0x81, 0x9a, 0xa3, 255 },  // #819aa3  Saturated Grey
            { 0xff, 0xff, 0xff, 255 },  // #ffffff  Saturated White
            { 0xde, 0x1a, 0x4d, 255 },  // #de1a4d  Saturated Red
            { 0xf2, 0x97, 0x34, 255 },  // #f29734  Saturated Orange
            { 0xe9, 0xcf, 0x1b, 255 },  // #e9cf1b  Saturated Yellow
            { 0xd4, 0xf0, 0x91, 255 },  // #d4f091  Saturated Light Green
            { 0x55, 0xe0, 0xc3, 255 },  // #55e0c3  Saturated Cyan
            { 0x72, 0xa0, 0xdd, 255 },  // #72a0dd  Saturated Light Blue
            { 0xe9, 0x97, 0xac, 255 },  // #e997ac  Saturated Light Red
            { 0xbe, 0xd0, 0xca, 255 }   // #bed0ca  Saturated Light Grey          
        }
    },
    [PALETTE_SONY_TRINITRON] = {
        .paletteName = "SONY-TRINITRON",
        .colorCount = 32,
        .colors = {
            { 0x08, 0x0A, 0x10, 255 },  // #080A10  BG_OBSIDIAN      : Global canvas black, transparency fill, absolute void
            { 0x1C, 0x20, 0x25, 255 },  // #1C2025  DARK_CHARCOAL    : Dark mode surface background, drop shadows, dark outlines
            { 0xA3, 0xA9, 0xB0, 255 },  // #A3A9B0  LIGHT_GREY       : Metallic highlights, light mode panels, unselected items
            { 0xC2, 0xC8, 0xD0, 255 },  // #C2C8D0  SILVER_TINT      : Bright light surfaces, disabled light mode button fills
            { 0x99, 0x00, 0x33, 255 },  // #990033  BURGUNDY_SHADOW  : Shadow steps for crimson or pink sprites, dark status fill
            { 0xFF, 0x00, 0x55, 255 },  // #FF0055  WALKMAN_PINK     : Primary brand action button, health bar fill, laser projectiles
            { 0x00, 0x5B, 0x66, 255 },  // #005B66  DEEP_SEA_CYAN    : Shadows for cyan tech sprites, empty magic/mana bar fill
            { 0x00, 0xE5, 0xFF, 255 },  // #00E5FF  ELECTRIC_TEAL    : Active status indicator toggles, mana meter fill, cyber lasers
            { 0x4A, 0x1A, 0x00, 255 },  // #4A1A00  MAROON_SHADOW    : Low-health empty bar indicator, dirt/rock shadow layers
            { 0x99, 0x33, 0x00, 255 },  // #993300  ALPHA_COPPER     : Heavy brick/earth textures, rusted metal plate tiles
            { 0x59, 0x44, 0x11, 255 },  // #594411  DARK_AMBER       : Treasure chest shadows, ancient architecture brick grids
            { 0xD4, 0xAF, 0x37, 255 },  // #D4AF37  NATIVE_GOLD      : Collected currency counts, legendary loot frames, active stars
            { 0x0A, 0x26, 0x12, 255 },  // #0A2612  FOREST_SHADOW    : Deep environment background foliage, shadow text outlines
            { 0x00, 0x66, 0x22, 255 },  // #006622  BROADCAST_GREEN  : Mid-tone grass tiles, basic flora grids, success message fills
            { 0x0F, 0x1A, 0x36, 255 },  // #0F1A36  MIDNIGHT_BLUE    : Dark sky backgrounds, deep ocean grids, storm level weather
            { 0x00, 0x37, 0xCD, 255 },  // #0037CD  PLAYSTATION_BLUE : Core UI menu banners, classic blue screens, water tiles

            { 0x4B, 0x4B, 0x4B, 255 },  // #4B4B4B  MID_GREY         : Window panels, interior grid lines, disabled text
            { 0x74, 0x7A, 0x80, 255 },  // #747A80  SLATE_GREY       : UI divider strokes, button borders, secondary text faces
            { 0xE6, 0xEC, 0xF2, 255 },  // #E6ECF2  OFF_WHITE        : Standard text face, primary crisp UI elements
            { 0xFF, 0xFF, 0xFF, 255 },  // #FFFFFF  PURE_WHITE       : Light beam peaks, text selections, flashes, and damage blink
            { 0xFF, 0x66, 0x99, 255 },  // #FF6699  PULSE_PINK       : Action button hover states, health bar glowing tube lines
            { 0xFF, 0xE5, 0xEE, 255 },  // #FFE5EE  PINK_TINT        : Neon pink reflections, heart icons, soft glow highlights
            { 0x66, 0xF3, 0xFF, 255 },  // #66F3FF  AQUA_MINT        : Mana bar glow line, neon typography accents, hover highlights
            { 0xE5, 0xFD, 0xFF, 255 },  // #E5FDFF  CYAN_TINT        : Ice effects, futuristic light reflections, soft glow windows
            { 0xFF, 0x45, 0x00, 255 },  // #FF4500  ALPHA_ORANGE     : Low-health warning bar, fiery explosions, lava tile loops
            { 0xFF, 0x99, 0x66, 255 },  // #FF9966  VIVID_COPPER     : Warning bar highlight steps, fire particle bursts
            { 0xFF, 0xE0, 0x66, 255 },  // #FFE066  TROPHY_GOLD      : Gold reflection highlights, critical hit popup text shadows
            { 0xFF, 0xF9, 0xE5, 255 },  // #FFF9E5  AMBER_CREAM      : Character dialogue backgrounds, scroll sheets, skin base
            { 0x00, 0xCC, 0x44, 255 },  // #00CC44  TRINITRON_GREEN  : Bright moss highlights, success notification checkmark icons
            { 0x66, 0xFF, 0x99, 255 },  // #66FF99  LIME_POP         : Emerald gems, glowing poison status elements, leaf highlights
            { 0x4A, 0x69, 0x84, 255 },  // #4A6984  SLATE_BLUE       : Steel weapon blades, storm clouds, passive navigation frames
            { 0xB0, 0xC4, 0xDE, 255 }   // #B0C4DE  ICY_BLUE         : Sky gradient highlights, mist, frost status indicator overlays
        }
    }
};

//    // SOFT CGA
//    // https://lospec.com/palette-list/soft-cga
//    [PALETTE_CGA] = {
//        .paletteName = "CGA",
//        .colorCount = 16,
//        .colors = {
//            { 0x00, 0x00, 0x00, 255 },  // #000000  Black
//            { 0x22, 0x00, 0x77, 255 },  // #220077  Blue            
//            { 0x77, 0x00, 0x88, 255 },  // #770088  Magenta
//            { 0x00, 0x77, 0x22, 255 },  // #007722  Green
//            { 0x99, 0x66, 0x00, 255 },  // #996600  Brown
//            { 0x44, 0x44, 0x55, 255 },  // #444455  Dark Grey
//            { 0x77, 0x77, 0x99, 255 },  // #777799  Grey
//            { 0xff, 0xff, 0xff, 255 },  // #ffffff  White
//            { 0x66, 0x00, 0x33, 255 },  // #660033  Red
//            { 0xff, 0x00, 0x33, 255 },  // #ff0033  Light Red
//            { 0xff, 0xee, 0x33, 255 },  // #ffee33  Yellow
//            { 0x00, 0xff, 0x33, 255 },  // #00ff33  Light Green
//            { 0x33, 0x22, 0xff, 255 },  // #3322ff  Light Blue
//            { 0x22, 0x77, 0x88, 255 },  // #227788  Cyan
//            { 0xff, 0x33, 0xdd, 255 },  // #ff33dd  Light Magenta
//            { 0x22, 0xee, 0xff, 255 }   // #22eeff  Light Cyan
//        }
//    }

// https://lospec.com/palette-list/ammo-8
// https://lospec.com/palette-list/andrade-gameboy
