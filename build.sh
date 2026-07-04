#!/bin/sh
set -e

ENABLE_DEBUG="0"

# Input files mapped to the new modular project layout
LIST_SRC_FILES="
./src/main.c
$(find ./src/apps -name "*.c" -type f)
$(find ./src/framework -name "*.c" -type f)"

FILE_OUT="picoray"

FLAG_DEBUG="-g"
PKGCONFIG_RAYLIB="$(pkg-config --libs --cflags raylib)"
PKGCONFIG_LUA="$(pkg-config --libs --cflags lua)"

if [[ "${ENABLE_DEBUG}" -eq 0 ]]; then
  FLAG_DEBUG=""
else
  FILE_OUT="${FILE_OUT}-debug"
fi

echo "# [1] Compiling PICO-RAY OS Core framework..."
echo "# eval cc ${ENABLE_DEBUG} $(echo "${LIST_SRC_FILES}" | tr "\n" " ") ${PKGCONFIG_RAYLIB} ${PKGCONFIG_LUA} -o ${FILE_OUT}"

eval cc \
  ${FLAG_DEBUG} \
  $(echo "${LIST_SRC_FILES}" | tr "\n" " ") \
  ${PKGCONFIG_RAYLIB} \
  ${PKGCONFIG_LUA} \
  -o ${FILE_OUT}

echo "# [2] Build successful: ./${FILE_OUT}"
