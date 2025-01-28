#include <SDL3/SDL.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "load.h"
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

  SDL_GPUShader *vertexShader = LoadShader(context.Device, "PositionColorInstanced.vert", 0, 0, 0, 0);
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

          .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{

              .format = SDL_GetGPUSwapchainTextureFormat(context.Device, context.Window)

          }},
      },
      // This is set up to match the vertex shader layout!
      .vertex_input_state = (SDL_GPUVertexInputState){

          .num_vertex_buffers = 1,

          .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{

              .slot = 0, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,

              .instance_step_rate = 0,

              .pitch = sizeof(PositionColorVertex)

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

                  .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,

                  .location = 1,

                  .offset = sizeof(float) * 3

              }

          }},
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

  // Create the vertex and index buffers
  SDL_GPUBuffer *VertexBuffer = SDL_CreateGPUBuffer(
      context.Device,
      &(SDL_GPUBufferCreateInfo){
          .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
          .size = sizeof(PositionColorVertex) * 9});

  SDL_GPUBuffer *IndexBuffer = SDL_CreateGPUBuffer(
      context.Device,
      &(SDL_GPUBufferCreateInfo){
          .usage = SDL_GPU_BUFFERUSAGE_INDEX,
          .size = sizeof(Uint16) * 6});

  // we will create a transfer buffer and write both the data that we want to transfer to the vertex & index buffers. We will just just an offset value.
  //  Set the buffer data
  SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(
      context.Device,
      &(SDL_GPUTransferBufferCreateInfo){
          .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
          .size = (sizeof(PositionColorVertex) * 9) + (sizeof(Uint16) * 6),
      });

  PositionColorVertex *transferData = SDL_MapGPUTransferBuffer(
      context.Device,
      transferBuffer,
      false);

  transferData[0] = (PositionColorVertex){-1, -1, 0, 255, 0, 0, 255};
  transferData[1] = (PositionColorVertex){1, -1, 0, 0, 255, 0, 255};
  transferData[2] = (PositionColorVertex){0, 1, 0, 0, 0, 255, 255};

  transferData[3] = (PositionColorVertex){-1, -1, 0, 255, 165, 0, 255};
  transferData[4] = (PositionColorVertex){1, -1, 0, 0, 128, 0, 255};
  transferData[5] = (PositionColorVertex){0, 1, 0, 0, 255, 255, 255};

  transferData[6] = (PositionColorVertex){-1, -1, 0, 255, 255, 255, 255};
  transferData[7] = (PositionColorVertex){1, -1, 0, 255, 255, 255, 255};
  transferData[8] = (PositionColorVertex){0, 1, 0, 255, 255, 255, 255};

  Uint16 *indexData = (Uint16 *)&transferData[9];
  for (Uint16 i = 0; i < 6; i += 1)
  {
    indexData[i] = i;
  }

  SDL_UnmapGPUTransferBuffer(context.Device, transferBuffer);

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
          .size = sizeof(PositionColorVertex) * 9},
      false);

  SDL_UploadToGPUBuffer(
      copyPass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = transferBuffer,
          // the offset that I was talking about
          .offset = sizeof(PositionColorVertex) * 9},
      &(SDL_GPUBufferRegion){
          .buffer = IndexBuffer,
          .offset = 0,
          .size = sizeof(Uint16) * 6},
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
      SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
      return -1;
    }

    SDL_GPUTexture *swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context.Window, &swapchainTexture, NULL, NULL))
    {
      SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
      return -1;
    }
    if (swapchainTexture == NULL)
      continue;

    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f};
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, context.Pipeline);
    SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){.buffer = VertexBuffer, .offset = 0}, 1);

    SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){.buffer = IndexBuffer, .offset = 0}, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    SDL_DrawGPUIndexedPrimitives(renderPass, 3, 16, 3, 0, 0);

    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
  }

  // cleanup
  SDL_ReleaseGPUBuffer(context.Device, VertexBuffer);
  SDL_ReleaseGPUBuffer(context.Device, IndexBuffer);

  SDL_ReleaseGPUGraphicsPipeline(context.Device, context.Pipeline);

  SDL_DestroyGPUDevice(context.Device);
  SDL_DestroyWindow(context.Window);
}