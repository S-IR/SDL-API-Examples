# Compiler settings
CC = gcc
CFLAGS = 
CLINK = -lSDL3

# Paths
SPV_BUILD_PATH = shader-binaries/spv
BUILD_DIR = build

# Project paths
HELLO_TRIANGLE_PATH = src/hello_triangle
RESIZE_PATH = src/resize
BASIC_VERTEX_PATH = src/basic_vertex_buffer
MANY_TRIANGLES_PATH = src/many_triangles
TEXTURE_QUAD_PATH = src/texture_quad
TEXTURE_ANIMATED_QUAD_PATH = src/texture_animated_quad
CUBE_PATH = src/cube

# Shader compiler
GLSLANG = glslangValidator

# Build configuration
USE_HLSL = true
USE_GLSL = true

# All targets
TARGETS = $(BUILD_DIR)/hello_triangle \
          $(BUILD_DIR)/resize \
          $(BUILD_DIR)/basic_vertex_buffer \
          $(BUILD_DIR)/many_triangles \
          $(BUILD_DIR)/texture_quad \
          $(BUILD_DIR)/texture_animated_quad \
          $(BUILD_DIR)/cube

.PHONY: all clean

all: $(BUILD_DIR) $(TARGETS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Hello Triangle
$(BUILD_DIR)/hello_triangle: $(HELLO_TRIANGLE_PATH)/hello_triangle.c
	@echo "Building Hello triangle"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -S vert -DVERTEX -V -o $(SPV_BUILD_PATH)/hello_triangle.vert.spv $(HELLO_TRIANGLE_PATH)/hello_triangle.glsl
	$(GLSLANG) -S frag -DFRAGMENT -V -o $(SPV_BUILD_PATH)/hello_triangle.frag.spv $(HELLO_TRIANGLE_PATH)/hello_triangle.glsl
endif
	$(CC) $< -o $@ $(CFLAGS) $(CLINK)

# Resize
$(BUILD_DIR)/resize: $(RESIZE_PATH)/resize.c
	@echo "Building Resize"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -e main -V $(RESIZE_PATH)/hlsl/RawTriangle.vert.hlsl -o $(SPV_BUILD_PATH)/RawTriangle.vert.spv
	$(GLSLANG) -e main -V $(RESIZE_PATH)/hlsl/SolidColor.frag.hlsl -o $(SPV_BUILD_PATH)/SolidColor.frag.spv
endif
	$(CC) $< -o $@ $(CFLAGS) $(CLINK)

# Basic Vertex Buffer
$(BUILD_DIR)/basic_vertex_buffer: $(BASIC_VERTEX_PATH)/basic_vertex_buffer.c
	@echo "Building basic vertex buffer"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -e main -V $(BASIC_VERTEX_PATH)/hlsl/PositionColor.vert.hlsl -o $(SPV_BUILD_PATH)/PositionColor.vert.spv
	$(GLSLANG) -e main -V $(BASIC_VERTEX_PATH)/hlsl/SolidColor.frag.hlsl -o $(SPV_BUILD_PATH)/SolidColor.frag.spv
endif
	$(CC) $< -o $@ $(CFLAGS) $(CLINK)

# Many Triangles
$(BUILD_DIR)/many_triangles: $(MANY_TRIANGLES_PATH)/many_triangles.c $(MANY_TRIANGLES_PATH)/load.c
	@echo "Building many triangles"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -e main -V $(MANY_TRIANGLES_PATH)/hlsl/PositionColorInstanced.vert.hlsl -o $(SPV_BUILD_PATH)/PositionColorInstanced.vert.spv
	$(GLSLANG) -e main -V $(MANY_TRIANGLES_PATH)/hlsl/SolidColor.frag.hlsl -o $(SPV_BUILD_PATH)/SolidColor.frag.spv
endif
	$(CC) $^ -o $@ $(CFLAGS) $(CLINK)

# Texture Quad
$(BUILD_DIR)/texture_quad: $(TEXTURE_QUAD_PATH)/texture_quad.c $(TEXTURE_QUAD_PATH)/load.c
	@echo "Building texture quad"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -e main -V $(TEXTURE_QUAD_PATH)/hlsl/TexturedQuad.vert.hlsl -o $(SPV_BUILD_PATH)/TexturedQuad.vert.spv
	$(GLSLANG) -e main -V $(TEXTURE_QUAD_PATH)/hlsl/TexturedQuad.frag.hlsl -o $(SPV_BUILD_PATH)/TexturedQuad.frag.spv
endif
ifeq ($(USE_GLSL), true)
	$(GLSLANG) -S vert -DVERTEX -V -o $(SPV_BUILD_PATH)/TexturedQuad.vert.spv $(TEXTURE_QUAD_PATH)/TexturedQuad.glsl
	$(GLSLANG) -S frag -DFRAGMENT -V -o $(SPV_BUILD_PATH)/TexturedQuad.frag.spv $(TEXTURE_QUAD_PATH)/TexturedQuad.glsl
endif
	$(CC) $^ -o $@ $(CFLAGS) $(CLINK)

# Texture Animated Quad
$(BUILD_DIR)/texture_animated_quad: $(TEXTURE_ANIMATED_QUAD_PATH)/texture_animated_quad.c $(TEXTURE_ANIMATED_QUAD_PATH)/load.c $(TEXTURE_ANIMATED_QUAD_PATH)/linear_algebra.c
	@echo "Building texture animated quad"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -e main -V $(TEXTURE_ANIMATED_QUAD_PATH)/hlsl/TexturedQuadWithMatrix.vert.hlsl -o $(SPV_BUILD_PATH)/TexturedQuadWithMatrix.vert.spv
	$(GLSLANG) -e main -V $(TEXTURE_ANIMATED_QUAD_PATH)/hlsl/TexturedQuadWithMultiplyColor.frag.hlsl -o $(SPV_BUILD_PATH)/TexturedQuadWithMultiplyColor.frag.spv
endif
ifeq ($(USE_GLSL), true)
	$(GLSLANG) -S vert -DVERTEX -V -o $(SPV_BUILD_PATH)/TexturedQuadWithMatrix.vert.spv $(TEXTURE_ANIMATED_QUAD_PATH)/TextureAnimatedQuad.glsl
	$(GLSLANG) -S frag -DFRAGMENT -V -o $(SPV_BUILD_PATH)/TexturedQuadWithMultiplyColor.frag.spv $(TEXTURE_ANIMATED_QUAD_PATH)/TextureAnimatedQuad.glsl
endif
	$(CC) $^ -o $@ $(CFLAGS) $(CLINK)

# Cube
$(BUILD_DIR)/cube: $(CUBE_PATH)/cube.c $(CUBE_PATH)/load.c $(CUBE_PATH)/linear_algebra.c
	@echo "Building cube"
ifeq ($(USE_HLSL), true)
	$(GLSLANG) -e main -V $(CUBE_PATH)/hlsl/PositionColorTransform.vert.hlsl -o $(SPV_BUILD_PATH)/PositionColorTransform.vert.spv
	$(GLSLANG) -e main -V $(CUBE_PATH)/hlsl/SolidColorDepth.frag.hlsl -o $(SPV_BUILD_PATH)/SolidColorDepth.frag.spv
	$(GLSLANG) -e main -V $(CUBE_PATH)/hlsl/TexturedQuad.vert.hlsl -o $(SPV_BUILD_PATH)/TexturedQuad.vert.spv
	$(GLSLANG) -e main -V $(CUBE_PATH)/hlsl/DepthOutline.frag.hlsl -o $(SPV_BUILD_PATH)/DepthOutline.frag.spv
endif
ifeq ($(USE_GLSL), true)
	$(GLSLANG) -S vert -DVERTEX -V -o $(SPV_BUILD_PATH)/PositionColorTransform.vert.spv $(CUBE_PATH)/cubeScene.glsl
	$(GLSLANG) -S frag -DFRAGMENT -V -o $(SPV_BUILD_PATH)/SolidColorDepth.frag.spv $(CUBE_PATH)/cubeScene.glsl
endif
	$(CC) $^ -o $@ $(CFLAGS) $(CLINK)

clean:
	rm -rf $(BUILD_DIR)