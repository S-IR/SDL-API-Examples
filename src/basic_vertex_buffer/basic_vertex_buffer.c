#include <SDL3/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
typedef struct Context
{
  SDL_GPUDevice *Device;
  SDL_Window *Window;
  SDL_GPUGraphicsPipeline *Pipeline;
} Context;

typedef struct PositionColorVertex
{
  float x, y, z;
  Uint8 r, g, b, a;
} PositionColorVertex;

float rand255()
{
  return (float)rand() / (float)(RAND_MAX) * 255.0f;
};
Context context = {0};
SDL_GPUShader *LoadShader(
    SDL_GPUDevice *device,
    const char *shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount);
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

  SDL_GPUShader *vertexShader = LoadShader(context.Device, "PositionColor.vert", 0, 0, 0, 0);
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

  // excuse the spacing. These are the remnants of a great war that was fought here against the c/c++ extension formatter in vscode
  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .target_info = {
          .num_color_targets = 1,
          .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
              .format = SDL_GetGPUSwapchainTextureFormat(context.Device, context.Window),
          }},
      },
      // This is set up to match the vertex shader layout!
      .vertex_input_state = (SDL_GPUVertexInputState){

          .num_vertex_buffers = 1,

          .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[])

              {{

                  .slot = 0,

                  .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,

                  .instance_step_rate = 0,

                  .pitch = sizeof(PositionColorVertex)

              }},

          .num_vertex_attributes = 2,

          .vertex_attributes = (SDL_GPUVertexAttribute[])

              {{

                   .buffer_slot = 0,

                   .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,

                   .location = 0,
                   .offset = 0

               },
               {

                   .buffer_slot = 0,

                   .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,

                   .location = 1,

                   .offset = sizeof(float) * 3

               }}

      },
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader

  };

  context.Pipeline = SDL_CreateGPUGraphicsPipeline(context.Device, &pipelineCreateInfo);
  if (context.Pipeline == NULL)
  {
    SDL_Log("Failed to create pipeline!");
    return -1;
  }

  SDL_ReleaseGPUShader(context.Device, vertexShader);
  SDL_ReleaseGPUShader(context.Device, fragmentShader);

  // Create the vertex buffer
  uint32_t gpuBufferSize = sizeof(PositionColorVertex) * 3;
  SDL_GPUBuffer *VertexBuffer = SDL_CreateGPUBuffer(
      context.Device,
      &(SDL_GPUBufferCreateInfo){
          .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
          .size = gpuBufferSize});

  // To get data into the vertex buffer, we have to use a transfer buffer
  SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(
      context.Device,
      &(SDL_GPUTransferBufferCreateInfo){
          .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
          .size = gpuBufferSize});

  // writing our data to the transfer buffer
  PositionColorVertex *transferData = SDL_MapGPUTransferBuffer(
      context.Device,
      transferBuffer,
      false);

  srand(time(NULL));

  transferData[0] = (PositionColorVertex){-1, -1, 0, rand255(), rand255(), rand255(), 255};
  transferData[1] = (PositionColorVertex){1, -1, 0, rand255(), rand255(), rand255(), 255};
  transferData[2] = (PositionColorVertex){0, 1, 0, rand255(), rand255(), rand255(), 255};

  SDL_UnmapGPUTransferBuffer(context.Device, transferBuffer);

  // then we order a copy command
  SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context.Device);
  SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

  SDL_UploadToGPUBuffer(
      copyPass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = transferBuffer,
          .offset = 0},
      &(SDL_GPUBufferRegion){
          .buffer = VertexBuffer,
          .offset = 0,
          .size = sizeof(PositionColorVertex) * 3},
      false);

  SDL_EndGPUCopyPass(copyPass);
  SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
  SDL_ReleaseGPUTransferBuffer(context.Device, transferBuffer);

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
    SDL_BindGPUGraphicsPipeline(renderPass, context.Pipeline);
    SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){.buffer = VertexBuffer, .offset = 0}, 1);
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }

  // cleanup
  SDL_ReleaseGPUBuffer(context.Device, VertexBuffer);
  SDL_ReleaseGPUGraphicsPipeline(context.Device, context.Pipeline);

  SDL_DestroyGPUDevice(context.Device);
  SDL_DestroyWindow(context.Window);
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
