#version 430 core

layout(std430, binding = 0) buffer vertPosBuffer {
    vec4 positions[];
};

layout(std430, binding = 1) buffer normalsBuffer  {
    vec4 normals[];
};

out vec3 vNormal;
out vec3 fragPos;   

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 pos = positions[gl_VertexID].xyz;
    vec3 normal = normals[gl_VertexID].xyz;

    gl_Position = projection * view * model * vec4(pos, 1.0);
    vNormal = mat3(transpose(inverse(model))) * normal;
    fragPos = vec3(model * vec4(pos, 1.0));
}
