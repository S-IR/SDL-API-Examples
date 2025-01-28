#version 450

#ifdef VERTEX
layout(set = 1, binding = 0) uniform UniformBlock
{
    mat4 MatrixTransform; // Transformation matrix
};

layout(location = 0) in vec4 inPosition; // Vertex position
layout(location = 1) in vec2 inTexCoord; // Texture coordinates

layout(location = 0) out vec2 outTexCoord;

void main()
{
    outTexCoord = inTexCoord;
    gl_Position = MatrixTransform * inPosition; // Transform vertex position
}
#endif

#ifdef FRAGMENT
layout(set = 3, binding = 0) uniform UniformBlock
{
    vec4 MultiplyColor; // Color multiplier
};

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;

void main()
{
    outColor = MultiplyColor * texture(texSampler, inTexCoord); // Sample texture and apply color multiplier
}
#endif