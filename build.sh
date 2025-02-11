#!/bin/bash
## YOU NEED glslangValidator INSTALLED TO COMPILE THE SHADERS


NC='\033[0m'
GREEN='\033[0;32m'

CC="gcc"
CFLAGS=""
CLINK="-lSDL3"

echo "Starting to build"
SPV_BUILD_PATH="shader-binaries/spv"

use_hlsl=false 
# glsl's are generated last so hlsl generated spirv will be overwritten
use_glsl=true


mkdir build/

echo -e "$GREEN   Building Hello triangle $NC"
HELLO_TRIANGLE_PATH="src/hello_triangle"

if $use_hlsl; then
  glslangValidator -S vert -DVERTEX -V -o $SPV_BUILD_PATH/hello_triangle.vert.spv $HELLO_TRIANGLE_PATH/hello_triangle.glsl
  glslangValidator -S frag -DFRAGMENT -V -o $SPV_BUILD_PATH/hello_triangle.frag.spv $HELLO_TRIANGLE_PATH/hello_triangle.glsl
fi
$CC  $HELLO_TRIANGLE_PATH/hello_triangle.c -o ./build/hello_triangle $CFLAGS $CLINK

RESIZE_PATH="src/resize"
echo -e "$GREEN   Building Resize $NC"
if $use_hlsl; then
  glslangValidator -e main -V $RESIZE_PATH/hlsl/RawTriangle.vert.hlsl -o $SPV_BUILD_PATH/RawTriangle.vert.spv
  glslangValidator -e main -V $RESIZE_PATH/hlsl/SolidColor.frag.hlsl -o $SPV_BUILD_PATH/SolidColor.frag.spv
fi
$CC  $RESIZE_PATH/resize.c -o ./build/resize $CFLAGS $CLINK

# echo "$CC $CFLAGS $CLINK $RESIZE_PATH/resize.c -o ./build/resize"

BASIC_VERTEX_PATH="src/basic_vertex_buffer"
echo -e "$GREEN   Building basic vertex buffer $NC"
if $use_hlsl; then
  glslangValidator -e main -V $BASIC_VERTEX_PATH/hlsl/PositionColor.vert.hlsl -o $SPV_BUILD_PATH/PositionColor.vert.spv
  glslangValidator -e main -V $BASIC_VERTEX_PATH/hlsl/SolidColor.frag.hlsl -o $SPV_BUILD_PATH/SolidColor.frag.spv
fi
$CC  $BASIC_VERTEX_PATH/basic_vertex_buffer.c -o ./build/basic_vertex_buffer $CFLAGS $CLINK


MANY_TRIANGLES_PATH="src/many_triangles"
echo -e "$GREEN  Building many triangles $NC"
if $use_hlsl; then
  glslangValidator -e main -V $MANY_TRIANGLES_PATH/hlsl/PositionColorInstanced.vert.hlsl -o $SPV_BUILD_PATH/PositionColorInstanced.vert.spv
  glslangValidator -e main -V $MANY_TRIANGLES_PATH/hlsl/SolidColor.frag.hlsl -o $SPV_BUILD_PATH/SolidColor.frag.spv
fi
$CC  $MANY_TRIANGLES_PATH/many_triangles.c $MANY_TRIANGLES_PATH/load.c -o ./build/many_triangles $CFLAGS $CLINK


TEXTURE_QUAD_PATH="src/texture_quad"
echo -e "$GREEN  Building texture quad $NC"
if $use_hlsl; then
  glslangValidator -e main -V $TEXTURE_QUAD_PATH/hlsl/TexturedQuad.vert.hlsl -o $SPV_BUILD_PATH/TexturedQuad.vert.spv
  glslangValidator -e main -V $TEXTURE_QUAD_PATH/hlsl/TexturedQuad.frag.hlsl -o $SPV_BUILD_PATH/TexturedQuad.frag.spv
fi

if $use_glsl; then
 glslangValidator -S vert -DVERTEX -V -o $SPV_BUILD_PATH/TexturedQuad.vert.spv $TEXTURE_QUAD_PATH/TexturedQuad.glsl
  glslangValidator -S frag -DFRAGMENT -V -o $SPV_BUILD_PATH/TexturedQuad.frag.spv $TEXTURE_QUAD_PATH/TexturedQuad.glsl
fi
$CC  $TEXTURE_QUAD_PATH/texture_quad.c $TEXTURE_QUAD_PATH/load.c -o ./build/texture_quad $CFLAGS $CLINK




TEXTURE_ANIMATED_QUAD_PATH="src/texture_animated_quad"
echo -e "$GREEN  Building texture animated quad $NC"
if $use_hlsl; then
  glslangValidator -e main -V $TEXTURE_ANIMATED_QUAD_PATH/hlsl/TexturedQuadWithMatrix.vert.hlsl -o $SPV_BUILD_PATH/TexturedQuadWithMatrix.vert.spv
  glslangValidator -e main -V $TEXTURE_ANIMATED_QUAD_PATH/hlsl/TexturedQuadWithMultiplyColor.frag.hlsl -o $SPV_BUILD_PATH/TexturedQuadWithMultiplyColor.frag.spv
fi

if $use_glsl; then
 glslangValidator -S vert -DVERTEX -V -o $SPV_BUILD_PATH/TexturedQuadWithMatrix.vert.spv $TEXTURE_ANIMATED_QUAD_PATH/TextureAnimatedQuad.glsl
  glslangValidator -S frag -DFRAGMENT -V -o $SPV_BUILD_PATH/TexturedQuadWithMultiplyColor.frag.spv $TEXTURE_ANIMATED_QUAD_PATH/TextureAnimatedQuad.glsl
fi
$CC  $TEXTURE_ANIMATED_QUAD_PATH/texture_animated_quad.c $TEXTURE_ANIMATED_QUAD_PATH/load.c $TEXTURE_ANIMATED_QUAD_PATH/linear_algebra.c -o ./build/texture_animated_quad $CFLAGS $CLINK



CUBE_PATH="src/cube"
echo -e "$GREEN  Building cube $NC"
if $use_hlsl; then
  glslangValidator -e main -V $CUBE_PATH/hlsl/PositionColorTransform.vert.hlsl -o $SPV_BUILD_PATH/PositionColorTransform.vert.spv
  glslangValidator -e main -V $CUBE_PATH/hlsl/SolidColorDepth.frag.hlsl -o $SPV_BUILD_PATH/SolidColorDepth.frag.spv

  glslangValidator -e main -V $CUBE_PATH/hlsl/TexturedQuad.vert.hlsl -o $SPV_BUILD_PATH/TexturedQuad.vert.spv
  glslangValidator -e main -V $CUBE_PATH/hlsl/DepthOutline.frag.hlsl -o $SPV_BUILD_PATH/DepthOutline.frag.spv
fi

if $use_glsl; then
 glslangValidator -S vert -DVERTEX -V -o $SPV_BUILD_PATH/PositionColorTransform.vert.spv $CUBE_PATH/cubeScene.glsl
  glslangValidator -S frag -DFRAGMENT -V -o $SPV_BUILD_PATH/SolidColorDepth.frag.spv $CUBE_PATH/cubeScene.glsl
fi
$CC  $CUBE_PATH/cube.c $CUBE_PATH/load.c $CUBE_PATH/linear_algebra.c -o ./build/cube $CFLAGS $CLINK
