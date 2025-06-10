#version 330 core
in vec4 ParticleColor;
in vec3 FragPos;

out vec4 color;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight sun;

void main()
{
    float aboveHorizon = smoothstep(-0.1, 0.05, dot(-sun.direction, vec3(0.0, 1.0, 0.0)));

    // Ambient
    vec3 ambient = sun.ambient * ParticleColor.rgb * aboveHorizon * 5.0;

    color = vec4(ambient, ParticleColor.w);
}  