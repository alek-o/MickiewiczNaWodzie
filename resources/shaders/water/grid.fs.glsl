#version 330 core

in vec3 vNormal;
in vec3 fragPos;

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

uniform DirLight sun;
uniform DirLight moon;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 sunLightDir = normalize(-sun.direction);
    vec3 moonLightDir = normalize(-moon.direction);

    float sunAboveHorizon = smoothstep(-0.1, 0.05, dot(-sun.direction, vec3(0.0, 1.0, 0.0)));
    float moonAboveHorizon = smoothstep(-0.1, 0.05, dot(-moon.direction, vec3(0.0, 1.0, 0.0)));

    // Ambient
    vec3 sunAmbient = sun.ambient * sunAboveHorizon;
    vec3 moonAmbient = moon.ambient * moonAboveHorizon;

    vec3 ambient = sunAmbient + moonAmbient;

    // Diffuse
    float sunDiff = max(dot(norm, sunLightDir), 0.0);
    vec3 sunDiffuse = sun.diffuse * sunDiff * sunAboveHorizon;
    float moonDiff = max(dot(norm, moonLightDir), 0.0);
    vec3 moonDiffuse = moon.diffuse * moonDiff * moonAboveHorizon;

    vec3 diffuse = sunDiffuse + moonDiffuse;

    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 sunReflectDir = reflect(-sunLightDir, norm);
    vec3 moonReflectDir = reflect(-moonLightDir, norm);
    
    float sunSpec = pow(max(dot(viewDir, sunReflectDir), 0.0), 64.0);
    vec3 sunSpecular = sun.specular * sunSpec * sunAboveHorizon * 0.5;
    float moonSpec = pow(max(dot(viewDir, moonReflectDir), 0.0), 64.0);
    vec3 moonSpecular = moon.specular * moonSpec * moonAboveHorizon * 0.3;

    vec3 specular = sunSpecular + moonSpecular;

    // Final color
    vec3 baseColor = vec3(0.35, 0.65, 0.9); // Water blue
    vec3 result = (ambient + diffuse) * baseColor + specular;
    FragColor = vec4(result, 1.0);
}