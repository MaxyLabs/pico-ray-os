#!/bin/sh

set -e  # Stop compilation immediately on any error

# Native core files required for the third-party dylib compilation
LIST_SRC_FILES="
$(find .src./apps -name "app_custom_cartridge.c" -type f)
$(find ./src/framework -name "*.c" -type f)"

PKGCONFIG_RAYLIB="$(pkg-config --libs --cflags raylib)"
PKGCONFIG_LUA="$(pkg-config --libs --cflags lua)"

# Set explicit output filename matching your custom cart designation
FILE_OUT="custom_game.dylib"

echo "# [1] Compiling Shared Library (.dylib) with Dynamic Lookup..."
echo "# cc -dynamiclib $(echo "${LIST_SRC_FILES}" | tr "\n" " ") -undefined dynamic_lookup ${PKGCONFIG_RAYLIB} ${PKGCONFIG_LUA} -o carts/${FILE_OUT}"

eval cc -dynamiclib $(echo "${LIST_SRC_FILES}" | tr "\n" " ") \
  -undefined dynamic_lookup \
  ${PKGCONFIG_RAYLIB} \
  ${PKGCONFIG_LUA} \
  -o carts/${FILE_OUT}

echo "# [2] Code Signing Shared Library for Apple Silicon Architecture..."
echo "# codesign --force --deep --sign - carts/${FILE_OUT}"
codesign --force --deep --sign - carts/${FILE_OUT}

echo "# [3] Checking Exported Symbols..."
nm -gU carts/${FILE_OUT}
