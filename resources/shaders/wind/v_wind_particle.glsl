#version 330 core
layout (location = 0) in vec3 aPos;

out vec4 ParticleColor;
out vec3 FragPos;

uniform vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));

    //float scale = 10.0f;
    ParticleColor = color;
    //gl_Position = projection * view * model * vec4(aPos * scale, 1.0f);
    gl_Position = projection * view * model * vec4(aPos, 1.0f);

}