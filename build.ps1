# Set variables
$CC = "gcc"
$CFLAGS = @("-Wall", "-O2", "-I'D:/utils/msys64/mingw64/include'", "-L'C:/Libraries/lib'", "-lSDL3")
$BUILD_DIR = "build"

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
& dxc -T ps_6_0 -E main -Fo  .\shader-binaries\dxil\SolidColor.frag.dxil .\src\resize\SolidColor.frag.hlsl
& dxc -T vs_6_0 -E main -Fo  .\shader-binaries\dxil\RawTriangle.vert.dxil .\src\resize\RawTriangle.vert.hlsl

& $CC $LDFLAGS $CFLAGS "src/resize/resize.c" -o "$BUILD_DIR/resize.exe" 

Write-Host "Build completed successfully!"
