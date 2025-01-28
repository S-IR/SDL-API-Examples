#include <SDL3/SDL.h>
#include <assert.h>

typedef struct Resolution
{
  uint32_t x;
  uint32_t y;
} Resolution;

typedef struct Context
{
  SDL_GPUDevice *Device;
  SDL_Window *Window;
  SDL_GPUGraphicsPipeline *Pipeline;
} Context;
const Resolution Resolutions[] =
    {
        {640, 480},
        {1280, 720},
        {1024, 1024},
        {1600, 900},
        {1920, 1080},
        {3200, 1800},
        {3840, 2160}};
uint32_t ResolutionCount = SDL_arraysize(Resolutions);

Sint32 ResolutionIndex;
// This load shader is different from hello_triangle shader in that it accepts only the name of the shader binary and it will fill the rest of the path
SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    const char *shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount);

Context context = {0};

void Cleanup();

int main(int argc, char const *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) == false)
  {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  context.Device = SDL_CreateGPUDevice(
      SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL,
      false,
      NULL);

  if (context.Device == NULL)
  {
    SDL_Log("GPUCreateDevice failed");
    return -1;
  }

  context.Window = SDL_CreateWindow("Basic Triangle", 640, 480, 0);
  if (context.Window == NULL)
  {
    SDL_Log("CreateWindow failed: %s", SDL_GetError());
    return -1;
  }

  if (!SDL_ClaimWindowForGPUDevice(context.Device, context.Window))
  {
    SDL_Log("GPUClaimWindow failed");
    return -1;
  }

  SDL_GPUShader *vertexShader = LoadShader(context.Device, "RawTriangle.vert", 0, 0, 0, 0);
  if (vertexShader == NULL)
  {
    SDL_Log("Failed to create vertex shader!");
    return -1;
  }

  SDL_GPUShader *fragmentShader = LoadShader(context.Device, "SolidColor.frag", 0, 0, 0, 0);
  if (fragmentShader == NULL)
  {
    SDL_Log("Failed to create fragment shader!");
    return -1;
  }

  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .target_info = {
          .num_color_targets = 1,
          .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{.format = SDL_GetGPUSwapchainTextureFormat(context.Device, context.Window)}},
      },
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader,
      .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL};
  SDL_GPUGraphicsPipeline *Pipeline = SDL_CreateGPUGraphicsPipeline(context.Device, &pipelineCreateInfo);
  if (Pipeline == NULL)
  {
    SDL_Log("Failed to create pipeline!");
    return -1;
  }

  SDL_ReleaseGPUShader(context.Device, vertexShader);
  SDL_ReleaseGPUShader(context.Device, fragmentShader);

  SDL_Event event;
  int quit = 0;

  while (!quit)
  {
    bool changeResolution = false;

    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT:
        quit = true;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_LEFT)
        {
          ResolutionIndex -= 1;
          if (ResolutionIndex < 0)
          {
            ResolutionIndex = ResolutionCount - 1;
          }
          changeResolution = true;
        }
        else if (event.key.key == SDLK_RIGHT)
        {
          SDL_Log("res count %d", ResolutionCount);
          ResolutionIndex = (ResolutionIndex + 1) % ResolutionCount;
          changeResolution = true;
        }

        if (changeResolution)
        {
          Resolution currentResolution = Resolutions[ResolutionIndex];
          SDL_Log("Setting resolution to: %u, %u", currentResolution.x, currentResolution.y);
          SDL_SetWindowSize(context.Window, currentResolution.x, currentResolution.y);
          SDL_SetWindowPosition(context.Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
          SDL_SyncWindow(context.Window);
        }
      }
    }

    SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(context.Device);
    if (cmdbuf == NULL)
    {
      SDL_Log("Aquire GPU command buffer failed :%s", SDL_GetError());
      return -1;
    }

    SDL_GPUTexture *swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.Window, &swapchainTexture, NULL, NULL))
    {
      SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
      return -1;
    }

    SDL_assert(swapchainTexture != NULL);

    SDL_GPUColorTargetInfo colorTargetInfo = {
        .texture = swapchainTexture,
        .clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE};

    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
    SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }
  Cleanup();
  return 0;
}

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
void Cleanup()
{
  if (context.Device != NULL && context.Pipeline != NULL)
  {
    SDL_ReleaseGPUGraphicsPipeline(context.Device, context.Pipeline);
  }
  if (context.Device != NULL)
  {
    SDL_DestroyWindow(context.Window);
    SDL_DestroyGPUDevice(context.Device);
  }
}
