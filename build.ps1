

# Set variables
$CC = "gcc"
$CFLAGS = @("-Wall", "-O2", "-I'D:/utils/msys64/mingw64/include'", "-L'C:/Libraries/lib'", "-lSDL3")
$BUILD_DIR = "build"
$COMPILE_DXIL = $true;
$COMPILE_SPIRV = $true;

$HLSL_OUTPUT_PATH = ".\shader-binaries\dxil"
$SPIRV_OUTPUT_PATH = ".\shader-binaries\spv"

# Create build directory if it doesn't exist
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Force -Path $BUILD_DIR
}

# Compile triangle
Write-Host "Compiling triangle..."
& $CC $CFLAGS "src/hello_triangle.c" -o "$BUILD_DIR/triangle.exe" 

# Compile shaders

# # Compile resize program
Write-Host "Compiling resize..."
$RESIZE_PATH = ".\src\resize"
if ($COMPILE_DXIL) {
    & dxc -T ps_6_0 -E main -Fo  $HLSL_OUTPUT_PATH\SolidColor.frag.dxil $RESIZE_PATH\hlsl\SolidColor.frag.hlsl
    & dxc -T vs_6_0 -E main -Fo  $HLSL_OUTPUT_PATH\RawTriangle.vert.dxil $RESIZE_PATH\hlsl\RawTriangle.vert.hlsl
}

if ($COMPILE_SPIRV) {
    glslangValidator -e main -V $RESIZE_PATH\hlsl\RawTriangle.vert.hlsl -o $SPIRV_OUTPUT_PATH\RawTriangle.vert.spv
    glslangValidator -e main -V $RESIZE_PATH\hlsl\SolidColor.frag.hlsl -o $SPIRV_OUTPUT_PATH\SolidColor.frag.spv
}
& $CC $LDFLAGS $CFLAGS "$RESIZE_PATH\resize.c" -o "$BUILD_DIR\resize.exe" 

Write-Host "Build completed successfully!"
