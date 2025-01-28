#version 450 core

#ifdef VERTEX

layout(location = 0) out vec4 v_color;
const vec2 positions[3] = vec2[3](
    vec2( 0.0,  0.5),
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5)
);
const vec4 colors[3] = vec4[3](
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
);
void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    v_color = colors[gl_VertexIndex];
}

#endif

#ifdef FRAGMENT

layout(location = 0) in vec4 v_color;
layout(location = 0) out vec4 o_color;
void main() {
    o_color = v_color;
}
#endif