#include <SDL3/SDL.h>
#include "load.h"
#include <stdio.h>

SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    const char *shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount)
{
  const char *ShaderBinaryBasePath = "./shader-binaries";

  // Auto-detect the shader stage from the file name for convenience
  SDL_GPUShaderStage stage;
  if (SDL_strstr(shaderFilename, ".vert"))
  {
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  }
  else if (SDL_strstr(shaderFilename, ".frag"))
  {
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  }
  else
  {
    SDL_Log("Invalid shader stage!");
    return NULL;
  }

  char fullPath[1024];
  SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
  SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
  const char *entrypoint;

  if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
  {
    SDL_snprintf(fullPath, sizeof(fullPath), "%s/spv/%s.spv", ShaderBinaryBasePath, shaderFilename);
    format = SDL_GPU_SHADERFORMAT_SPIRV;
    entrypoint = "main";
  }
  else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
  {
    SDL_snprintf(fullPath, sizeof(fullPath), "%s/msl/%s.msl", ShaderBinaryBasePath, shaderFilename);
    format = SDL_GPU_SHADERFORMAT_MSL;
    entrypoint = "main0";
  }
  else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
  {
    SDL_snprintf(fullPath, sizeof(fullPath), "%s/dxil/%s.dxil", ShaderBinaryBasePath, shaderFilename);
    format = SDL_GPU_SHADERFORMAT_DXIL;
    entrypoint = "main";
  }
  else
  {
    SDL_Log("%s", "Unrecognized backend shader format!");
    return NULL;
  }

  size_t codeSize;
  void *code = SDL_LoadFile(fullPath, &codeSize);
  if (code == NULL)
  {
    SDL_Log("Failed to load shader from disk! %s", fullPath);
    return NULL;
  }

  SDL_GPUShaderCreateInfo shaderInfo = {
      .code = code,
      .code_size = codeSize,
      .entrypoint = entrypoint,
      .format = format,
      .stage = stage,
      .num_samplers = samplerCount,
      .num_uniform_buffers = uniformBufferCount,
      .num_storage_buffers = storageBufferCount,
      .num_storage_textures = storageTextureCount};
  SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shaderInfo);
  if (shader == NULL)
  {
    SDL_Log("Failed to create shader!");
    SDL_free(code);
    return NULL;
  }
  SDL_free(code);
  return shader;
}

SDL_Surface *LoadImage(const char *imageFilename, int desiredChannels)
{
  const char *BasePath = "images";
  char fullPath[256];
  SDL_Surface *result;
  SDL_PixelFormat format;

  SDL_snprintf(fullPath, sizeof(fullPath), "%s/%s", BasePath, imageFilename);

  result = SDL_LoadBMP(fullPath);
  if (result == NULL)
  {
    SDL_Log("Failed to load BMP: %s", SDL_GetError());
    return NULL;
  }

  if (desiredChannels == 4)
  {
    format = SDL_PIXELFORMAT_ABGR8888;
  }
  else
  {
    SDL_assert(!"Unexpected desiredChannels");
    SDL_DestroySurface(result);
    return NULL;
  }
  if (result->format != format)
  {
    SDL_Surface *next = SDL_ConvertSurface(result, format);
    SDL_DestroySurface(result);
    result = next;
  }

  return result;
}