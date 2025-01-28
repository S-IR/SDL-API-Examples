#version 450 core

#ifdef VERTEX

// Input vertex data
layout(location = 0) in vec3 inPosition;  // Vertex position
layout(location = 1) in vec2 inTexCoord;  // Texture coordinates

// Output to the fragment shader
layout(location = 0) out vec2 outTexCoord;

void main()
{
    // Pass texture coordinates to the fragment shader
    outTexCoord = inTexCoord;

    // Transform the vertex position to clip space
    gl_Position = vec4(inPosition, 1.0);
}

#endif

#ifdef FRAGMENT

// Input from the vertex shader
layout(location = 0) in vec2 inTexCoord;

// Output to the framebuffer
layout(location = 0) out vec4 outColor;

// Texture and sampler
layout(set = 2, binding = 0) uniform sampler2D texSampler;

void main()
{
    // Sample the texture and output the color
    outColor = texture(texSampler, inTexCoord);
}

#endif
