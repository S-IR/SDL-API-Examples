#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef VERTEX

layout(set = 1, binding = 0) uniform UBO {
    mat4 transform;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
    gl_Position = transform * vec4(inPosition, 1.0);
}

#endif

#ifdef FRAGMENT
layout(set = 3, binding = 0) uniform UBO {
    float NearPlane;
    float FarPlane;
};

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out float outDepth;

float LinearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0;
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;
}

void main() {
    outColor = inColor;
    outDepth = LinearizeDepth(gl_FragCoord.z, NearPlane, FarPlane);
}
#endif