
#include <SDL3/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "load.h"

const char *SamplerNames[] =
    {
        "PointClamp",
        "PointWrap",
        "LinearClamp",
        "LinearWrap",
        "AnisotropicClamp",
        "AnisotropicWrap",
};
typedef struct Context
{
  SDL_GPUDevice *Device;
  SDL_Window *Window;
  SDL_GPUGraphicsPipeline *Pipeline;
} Context;
static SDL_GPUSampler *Samplers[SDL_arraysize(SamplerNames)];

typedef struct PositionColorVertex
{
  float x, y, z;
  Uint8 r, g, b, a;
} PositionColorVertex;
typedef struct PositionTextureVertex
{
  float x, y, z;
  float u, v;
} PositionTextureVertex;
float rand255()
{
  return (float)rand() / (float)(RAND_MAX) * 255.0f;
};
Context context = {0};

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

  context.Window = SDL_CreateWindow("Texture Quad", 640, 480, 0);
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
  // Create the shaders
  SDL_GPUShader *vertexShader = LoadShader(context.Device, "TexturedQuad.vert", 0, 0, 0, 0);
  if (vertexShader == NULL)
  {
    SDL_Log("Failed to create vertex shader!");
    return -1;
  }

  SDL_GPUShader *fragmentShader = LoadShader(context.Device, "TexturedQuad.frag", 1, 0, 0, 0);
  if (fragmentShader == NULL)
  {
    SDL_Log("Failed to create fragment shader!");
    return -1;
  }

  // Load the image
  SDL_Surface *imageData = LoadImage("ravioli.bmp", 4);
  if (imageData == NULL)
  {
    SDL_Log("Could not load image data!");
    return -1;
  }
  // Create the pipeline
  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .target_info = {

          .num_color_targets = 1,

          .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{

              .format = SDL_GetGPUSwapchainTextureFormat(context.Device, context.Window)

          }},

      },
      .vertex_input_state = (SDL_GPUVertexInputState){

          .num_vertex_buffers = 1,

          .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{

              .slot = 0,

              .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,

              .instance_step_rate = 0,

              .pitch = sizeof(PositionTextureVertex)

          }},
          .num_vertex_attributes = 2,

          .vertex_attributes = (SDL_GPUVertexAttribute[]){

              {

                  .buffer_slot = 0,

                  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                  .location = 0,

                  .offset = 0

              },
              {

                  .buffer_slot = 0,

                  .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,

                  .location = 1,

                  .offset = sizeof(float) * 3

              }}

      },
      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
      .vertex_shader = vertexShader,
      .fragment_shader = fragmentShader};

  context.Pipeline = SDL_CreateGPUGraphicsPipeline(context.Device, &pipelineCreateInfo);
  if (context.Pipeline == NULL)
  {
    SDL_Log("Failed to create pipeline!");
    return -1;
  }

  SDL_ReleaseGPUShader(context.Device, vertexShader);
  SDL_ReleaseGPUShader(context.Device, fragmentShader);

  // PointClamp
  Samplers[0] = SDL_CreateGPUSampler(context.Device, &(SDL_GPUSamplerCreateInfo){
                                                         .min_filter = SDL_GPU_FILTER_NEAREST,
                                                         .mag_filter = SDL_GPU_FILTER_NEAREST,
                                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
                                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                     });
  // PointWrap
  Samplers[1] = SDL_CreateGPUSampler(context.Device, &(SDL_GPUSamplerCreateInfo){
                                                         .min_filter = SDL_GPU_FILTER_NEAREST,
                                                         .mag_filter = SDL_GPU_FILTER_NEAREST,
                                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
                                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                     });
  // LinearClamp
  Samplers[2] = SDL_CreateGPUSampler(context.Device, &(SDL_GPUSamplerCreateInfo){
                                                         .min_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mag_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                     });
  // LinearWrap
  Samplers[3] = SDL_CreateGPUSampler(context.Device, &(SDL_GPUSamplerCreateInfo){
                                                         .min_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mag_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                     });
  // AnisotropicClamp
  Samplers[4] = SDL_CreateGPUSampler(context.Device, &(SDL_GPUSamplerCreateInfo){
                                                         .min_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mag_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                                                         .enable_anisotropy = true,
                                                         .max_anisotropy = 4});
  Samplers[5] = SDL_CreateGPUSampler(context.Device, &(SDL_GPUSamplerCreateInfo){
                                                         .min_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mag_filter = SDL_GPU_FILTER_LINEAR,
                                                         .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                                                         .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                                                         .enable_anisotropy = true,
                                                         .max_anisotropy = 4});
  // Create the GPU resources
  SDL_GPUBuffer *VertexBuffer = SDL_CreateGPUBuffer(
      context.Device,
      &(SDL_GPUBufferCreateInfo){
          .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
          .size = sizeof(PositionTextureVertex) * 4});
  SDL_SetGPUBufferName(
      context.Device,
      VertexBuffer,
      "Ravioli Vertex Buffer 🥣");

  SDL_GPUBuffer *IndexBuffer = SDL_CreateGPUBuffer(
      context.Device,
      &(SDL_GPUBufferCreateInfo){
          .usage = SDL_GPU_BUFFERUSAGE_INDEX,
          .size = sizeof(Uint16) * 6});

  SDL_GPUTexture *Texture = SDL_CreateGPUTexture(context.Device, &(SDL_GPUTextureCreateInfo){
                                                                     .type = SDL_GPU_TEXTURETYPE_2D,
                                                                     .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                                                                     .width = imageData->w,
                                                                     .height = imageData->h,
                                                                     .layer_count_or_depth = 1,
                                                                     .num_levels = 1,
                                                                     .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER});
  SDL_SetGPUTextureName(
      context.Device,
      Texture,
      "Ravioli Texture 🖼️");

  // Set up buffer data
  SDL_GPUTransferBuffer *bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
      context.Device,
      &(SDL_GPUTransferBufferCreateInfo){
          .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
          .size = (sizeof(PositionTextureVertex) * 4) + (sizeof(Uint16) * 6)});

  PositionTextureVertex *transferData = SDL_MapGPUTransferBuffer(
      context.Device,
      bufferTransferBuffer,
      false);

  transferData[0] = (PositionTextureVertex){-1, 1, 0, 0, 0};
  transferData[1] = (PositionTextureVertex){1, 1, 0, 4, 0};
  transferData[2] = (PositionTextureVertex){1, -1, 0, 4, 4};
  transferData[3] = (PositionTextureVertex){-1, -1, 0, 0, 4};

  Uint16 *indexData = (Uint16 *)&transferData[4];
  indexData[0] = 0;
  indexData[1] = 1;
  indexData[2] = 2;
  indexData[3] = 0;
  indexData[4] = 2;
  indexData[5] = 3;

  SDL_UnmapGPUTransferBuffer(context.Device, bufferTransferBuffer);

  // Set up texture data
  SDL_GPUTransferBuffer *textureTransferBuffer = SDL_CreateGPUTransferBuffer(
      context.Device,
      &(SDL_GPUTransferBufferCreateInfo){
          .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
          .size = imageData->w * imageData->h * 4});

  Uint8 *textureTransferPtr = SDL_MapGPUTransferBuffer(
      context.Device,
      textureTransferBuffer,
      false);
  SDL_memcpy(textureTransferPtr, imageData->pixels, imageData->w * imageData->h * 4);
  SDL_UnmapGPUTransferBuffer(context.Device, textureTransferBuffer);

  // Upload the transfer data to the GPU resources
  SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context.Device);
  SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

  SDL_UploadToGPUBuffer(
      copyPass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = bufferTransferBuffer,
          .offset = 0},
      &(SDL_GPUBufferRegion){
          .buffer = VertexBuffer,
          .offset = 0,
          .size = sizeof(PositionTextureVertex) * 4},
      false);

  SDL_UploadToGPUBuffer(
      copyPass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = bufferTransferBuffer,
          .offset = sizeof(PositionTextureVertex) * 4},
      &(SDL_GPUBufferRegion){
          .buffer = IndexBuffer,
          .offset = 0,
          .size = sizeof(Uint16) * 6},
      false);

  SDL_UploadToGPUTexture(
      copyPass,
      &(SDL_GPUTextureTransferInfo){
          .transfer_buffer = textureTransferBuffer,
          .offset = 0, /* Zeros out the rest */
      },
      &(SDL_GPUTextureRegion){
          .texture = Texture,
          .w = imageData->w,
          .h = imageData->h,
          .d = 1},
      false);

  SDL_EndGPUCopyPass(copyPass);
  SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
  SDL_DestroySurface(imageData);
  SDL_ReleaseGPUTransferBuffer(context.Device, bufferTransferBuffer);
  SDL_ReleaseGPUTransferBuffer(context.Device, textureTransferBuffer);

  // Finally, print instructions!
  SDL_Log("Press Left/Right to switch between sampler states");
  SDL_Log("Setting sampler state to: %s", SamplerNames[0]);

  SDL_Event event;
  int quit = 0;
  int CurrentSamplerIndex = 0;

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
          CurrentSamplerIndex -= 1;
          if (CurrentSamplerIndex < 0)
          {
            CurrentSamplerIndex = SDL_arraysize(Samplers) - 1;
          }
          SDL_Log("Setting sampler state to: %s", SamplerNames[CurrentSamplerIndex]);
        }
        else if (event.key.key == SDLK_RIGHT)
        {
          CurrentSamplerIndex = (CurrentSamplerIndex + 1) % SDL_arraysize(Samplers);
          SDL_Log("Setting sampler state to: %s", SamplerNames[CurrentSamplerIndex]);
        }
      }
    }
    SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(context.Device);
    if (cmdbuf == NULL)
    {
      SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
      return -1;
    }

    SDL_GPUTexture *swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.Window, &swapchainTexture, NULL, NULL))
    {
      SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
      return -1;
    }

    if (swapchainTexture != NULL)
    {
      SDL_GPUColorTargetInfo colorTargetInfo = {0};
      colorTargetInfo.texture = swapchainTexture;
      colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
      colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
      colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

      SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

      SDL_BindGPUGraphicsPipeline(renderPass, context.Pipeline);
      SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){.buffer = VertexBuffer, .offset = 0}, 1);
      SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){.buffer = IndexBuffer, .offset = 0}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
      SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){.texture = Texture, .sampler = Samplers[CurrentSamplerIndex]}, 1);
      SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);

      SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }
  for (int i = 0; i < SDL_arraysize(Samplers); i++)
  {
    SDL_ReleaseGPUSampler(context.Device, Samplers[i]);
  }
  // cleanup
  SDL_ReleaseGPUTexture(context.Device, Texture);
  SDL_ReleaseGPUBuffer(context.Device, VertexBuffer);
  SDL_ReleaseGPUBuffer(context.Device, IndexBuffer);

  SDL_ReleaseGPUGraphicsPipeline(context.Device, context.Pipeline);

  SDL_DestroyGPUDevice(context.Device);
  SDL_DestroyWindow(context.Window);
}