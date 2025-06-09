#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) buffer vertPosBuffer {
	vec4 positions[];
};

uniform float time;
uniform uint gridRes;

void main() {
	float freq = 0.5;
    float amp = 0.5;

    // Calculate global index for 2D grid (you need to match this on CPU side too)
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    uint index = y * gridRes + x;

    // Read current position
    vec3 pos = positions[index].xyz;
    float px = pos.x;
    float pz = pos.z;

    float height = sin(px * freq + time) * cos(pz * freq + time);
    pos = vec3(px, height * amp, pz);

    positions[index] = vec4(pos, 0.0);
}