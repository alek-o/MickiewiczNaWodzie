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
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(-sun.direction);

    // Ambient
    vec3 ambient = sun.ambient;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = sun.diffuse * diff;

    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = sun.specular * spec;

    // Final color
    vec3 baseColor = vec3(0.2, 0.5, 0.8); // Water blue
    vec3 result = (ambient + diffuse) * baseColor + specular;
    FragColor = vec4(result, 1.0);
}