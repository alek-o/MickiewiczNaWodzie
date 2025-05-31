#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main() {
    float freq = 0.5;
    float amp = 0.5;

    float height = sin(aPos.x * freq + time) * cos(aPos.z * freq + time);
    vec3 displacedPos = vec3(aPos.x, height * amp, aPos.z);

    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}
