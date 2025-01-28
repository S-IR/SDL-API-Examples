#ifndef LOAD_SHADER_H_
#define LOAD_SHADER_H_
#include <SDL3/SDL.h>

SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    const char *shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount);
#endif // LOAD_SHADER_H_