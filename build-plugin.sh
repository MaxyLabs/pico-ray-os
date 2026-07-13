#!/bin/sh

set -e  # Stop compilation immediately on any error

# FIX: Only compile the dedicated cartridge source file. 
# DO NOT bundle or re-compile the global framework .c source files into the dylib!
LIST_SRC_FILES="$(find ./src/apps -name "app_custom_cartridge.c" -type f)"

PKGCONFIG_RAYLIB="$(pkg-config --libs --cflags raylib)"
PKGCONFIG_LUA="$(pkg-config --libs --cflags lua)"

FILE_OUT="custom_game.dylib"

echo "# [1] Compiling Shared Library (.dylib) with Dynamic Lookup..."
cc -dynamiclib ${LIST_SRC_FILES} \
  -undefined dynamic_lookup \
  ${PKGCONFIG_RAYLIB} \
  ${PKGCONFIG_LUA} \
  -o carts/${FILE_OUT}

echo "# [2] Code Signing Shared Library for Apple Silicon Architecture..."
codesign --force --deep --sign - carts/${FILE_OUT}

echo "# [3] Checking Exported Symbols..."
nm -gU carts/${FILE_OUT}
