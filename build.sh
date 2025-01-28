#!/bin/bash
## YOU NEED glslangValidator INSTALLED TO COMPILE THE SHADERS


NC='\033[0m'
GREEN='\033[0;32m'

CC="gcc"
CFLAGS=""
CLINK="-lSDL3"

echo "Starting to build"
SPV_BUILD_PATH="shader-binaries/spv"

mkdir build/

echo -e "$GREEN   Building Hello triangle $NC"
HELLO_TRIANGLE_PATH="src/hello_triangle"
glslangValidator -S vert -DVERTEX -V -o $SPV_BUILD_PATH/hello_triangle.vert.spv $HELLO_TRIANGLE_PATH/hello_triangle.glsl
glslangValidator -S frag -DFRAGMENT -V -o $SPV_BUILD_PATH/hello_triangle.frag.spv $HELLO_TRIANGLE_PATH/hello_triangle.glsl
$CC  $HELLO_TRIANGLE_PATH/hello_triangle.c -o ./build/hello_triangle $CFLAGS $CLINK

RESIZE_PATH="src/resize"
echo -e "$GREEN   Building Resize $NC"
glslangValidator -e main -V $RESIZE_PATH/hlsl/RawTriangle.vert.hlsl -o $SPV_BUILD_PATH/RawTriangle.vert.spv
glslangValidator -e main -V $RESIZE_PATH/hlsl/SolidColor.frag.hlsl -o $SPV_BUILD_PATH/SolidColor.frag.spv
$CC  $RESIZE_PATH/resize.c -o ./build/resize $CFLAGS $CLINK
# echo "$CC $CFLAGS $CLINK $RESIZE_PATH/resize.c -o ./build/resize"
