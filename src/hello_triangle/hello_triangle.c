#include <SDL3/SDL.h>

typedef struct Context
{
  SDL_GPUDevice *Device;
  SDL_Window *Window;
  SDL_GPUGraphicsPipeline *Pipeline;
} Context;
Context context = {0};

void Cleanup();
SDL_GPUGraphicsPipeline *Pipeline;

SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    const char *shaderPath,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount)
{
  // Auto-detect the shader stage from the file name for convenience
  SDL_GPUShaderStage stage;
  if (SDL_strstr(shaderPath, ".vert"))
  {
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  }
  else if (SDL_strstr(shaderPath, ".frag"))
  {
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  }
  else
  {
    SDL_Log("Invalid shader stage!");
    return NULL;
  }

  SDL_GPUShaderFormat format = SDL_GetGPUShaderFormats(device);
  const char *entrypoint;

  if (format & SDL_GPU_SHADERFORMAT_SPIRV)
  {
    entrypoint = "main";
  }
  else if (format & SDL_GPU_SHADERFORMAT_MSL)
  {
    entrypoint = "main0";
  }
  else if (format & SDL_GPU_SHADERFORMAT_DXIL)
  {
    entrypoint = "main";
  }
  else
  {
    SDL_Log("%s", "Unrecognized backend shader format!");
    return NULL;
  }

  size_t codeSize;
  void *code = SDL_LoadFile(shaderPath, &codeSize);
  if (code == NULL)
  {
    SDL_Log("Failed to load shader from disk! %s", shaderPath);
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

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO) == false)
  {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  context.Device = SDL_CreateGPUDevice(
      SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
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

  SDL_GPUShader *vertexShader = LoadShader(context.Device, "shader-binaries/spv/hello_triangle.vert.spv", 0, 0, 0, 0);
  SDL_GPUShader *fragmentShader = LoadShader(context.Device, "shader-binaries/spv/hello_triangle.frag.spv", 0, 0, 0, 0);

  if (!vertexShader || !fragmentShader)
  {
    SDL_Log("Failed to create shaders!");
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
  };

  Pipeline = SDL_CreateGPUGraphicsPipeline(context.Device, &pipelineCreateInfo);
  if (Pipeline == NULL)
  {
    SDL_Log("Failed to create pipeline!");
    return -1;
  }

  SDL_ReleaseGPUShader(context.Device, vertexShader);
  SDL_ReleaseGPUShader(context.Device, fragmentShader);

  // Main loop
  SDL_Event event;
  int quit = 0;

  while (!quit)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        quit = 1;
      }
    }

    SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(context.Device);
    if (!cmdbuf)
      continue;

    SDL_GPUTexture *swapchainTexture;
    if (SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.Window, &swapchainTexture, NULL, NULL))
    {
      SDL_GPUColorTargetInfo colorTargetInfo = {
          .texture = swapchainTexture,
          .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
          .load_op = SDL_GPU_LOADOP_CLEAR,
          .store_op = SDL_GPU_STOREOP_STORE};

      SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
      SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
      SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
      SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }

  // Cleanup
  SDL_ReleaseGPUGraphicsPipeline(context.Device, Pipeline);
  Cleanup();
  SDL_Quit();

  return 0;
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
