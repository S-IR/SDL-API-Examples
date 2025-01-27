#include <SDL3/SDL.h>

typedef struct Context
{
  SDL_GPUDevice *Device;
  SDL_Window *Window;
} Context;

// Simple vertex shader that creates a triangle
const char *vertexShaderCode =
    "#version 450\n"
    "layout(location = 0) out vec4 v_color;\n"
    "const vec2 positions[3] = vec2[3](\n"
    "    vec2( 0.0,  0.5),\n"
    "    vec2(-0.5, -0.5),\n"
    "    vec2( 0.5, -0.5)\n"
    ");\n"
    "const vec4 colors[3] = vec4[3](\n"
    "    vec4(1.0, 0.0, 0.0, 1.0),\n"
    "    vec4(0.0, 1.0, 0.0, 1.0),\n"
    "    vec4(0.0, 0.0, 1.0, 1.0)\n"
    ");\n"
    "void main() {\n"
    "    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);\n"
    "    v_color = colors[gl_VertexIndex];\n"
    "}\n";

// Simple fragment shader that outputs the interpolated color
const char *fragmentShaderCode =
    "#version 450\n"
    "layout(location = 0) in vec4 v_color;\n"
    "layout(location = 0) out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

void CommonQuit(Context *context)
{
  SDL_ReleaseWindowFromGPUDevice(context->Device, context->Window);
  SDL_DestroyWindow(context->Window);
  SDL_DestroyGPUDevice(context->Device);
}

static SDL_GPUGraphicsPipeline *Pipeline;

static SDL_GPUShader *LoadShader(SDL_GPUDevice *device, SDL_GPUShaderStage stage, const char *code, size_t codeSize)
{
  SDL_GPUShaderCreateInfo shaderInfo = {
      .code = code,
      .code_size = codeSize,
      .entrypoint = "main",
      .format = SDL_GPU_SHADERFORMAT_SPIRV,
      .stage = stage};
  return SDL_CreateGPUShader(device, &shaderInfo);
}

int main(int argc, char **argv)
{
  SDL_Log("hello workd");

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  Context context = {0};
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

  SDL_GPUShader *vertexShader = LoadShader(context.Device, SDL_GPU_SHADERSTAGE_VERTEX,
                                           vertexShaderCode, SDL_strlen(vertexShaderCode));
  SDL_GPUShader *fragmentShader = LoadShader(context.Device, SDL_GPU_SHADERSTAGE_FRAGMENT,
                                             fragmentShaderCode, SDL_strlen(fragmentShaderCode));

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
  CommonQuit(&context);
  SDL_Quit();

  return 0;
}