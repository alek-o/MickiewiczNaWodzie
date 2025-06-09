#version 430 core
layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) readonly buffer VertexBuffer {
    vec4 positions[];
};

layout(std430, binding = 1) writeonly buffer NormalBuffer {
	vec4 normals[];
};

uniform uint gridRes; 

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    int x = int(id.x);
    int z = int(id.y);

    if (x >= gridRes || z >= gridRes) return;

    int idx = z * int(gridRes) + x;

    // Clamp edges
    int x0 = max(int(x) - 1, 0);
    int x1 = min(int(x) + 1, int(gridRes) - 1);
    int z0 = max(int(z) - 1, 0);
    int z1 = min(int(z) + 1, int(gridRes) - 1);

    vec3 pL = positions[z * gridRes + x0].xyz;
    vec3 pR = positions[z * gridRes + x1].xyz;
    vec3 pD = positions[z0 * gridRes + x].xyz;
    vec3 pU = positions[z1 * gridRes + x].xyz;

    vec3 dx = pR - pL;
    vec3 dz = pU - pD;

    vec3 normal = normalize(cross(dz, dx));

    normals[idx] = vec4(normal, 0.0);
}
