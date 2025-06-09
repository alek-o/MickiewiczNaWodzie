#version 430 core

layout(std430, binding = 0) buffer vertPosBuffer {
    vec4 positions[];
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 pos = positions[gl_VertexID].xyz;
    gl_Position = projection * view * model * vec4(pos, 1.0);
}
