static float4 Color;
static float4 _entryPointOutput;

struct SPIRV_Cross_Input
{
    float4 Color : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 _entryPointOutput : COLOR0;
};

void frag_main()
{
    _entryPointOutput = Color;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    Color = stage_input.Color;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output._entryPointOutput = float4(_entryPointOutput);
    return stage_output;
}
