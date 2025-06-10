#version 330 core

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform DirLight sun;
uniform DirLight moon;
uniform vec3 viewPos;


void main()
{
    vec3 norm = normalize(Normal);
    vec3 sunLightDir = normalize(-sun.direction);
    vec3 moonLightDir = normalize(-moon.direction);

    float sunAboveHorizon = smoothstep(-0.1, 0.05, dot(-sun.direction, vec3(0.0, 1.0, 0.0)));
    float moonAboveHorizon = smoothstep(-0.1, 0.05, dot(-moon.direction, vec3(0.0, 1.0, 0.0)));

    // Ambient
    vec3 sunAmbient = sun.ambient * texture(texture_diffuse1, TexCoord).rgb * sunAboveHorizon;
    vec3 moonAmbient = moon.ambient * texture(texture_diffuse1, TexCoord).rgb * moonAboveHorizon;
    
    vec3 ambient = sunAmbient + moonAmbient;
    
    // Diffuse
    float sunDiff = max(dot(norm, sunLightDir), 0.0);
    vec3 sunDiffuse = sun.diffuse * sunDiff * sunAboveHorizon * texture(texture_diffuse1, TexCoord).rgb;
    float moonDiff = max(dot(norm, moonLightDir), 0.0);
    vec3 moonDiffuse = moon.diffuse * moonDiff * moonAboveHorizon * texture(texture_diffuse1, TexCoord).rgb;

    vec3 diffuse = sunDiffuse + moonDiffuse;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 sunReflectDir = reflect(-sunLightDir, norm);
    vec3 moonReflectDir = reflect(-moonLightDir, norm);

    float sunSpec = pow(max(dot(viewDir, sunReflectDir), 0.0), 64.0);
    vec3 sunSpecular = sun.specular * sunSpec * sunAboveHorizon;
    float moonSpec = pow(max(dot(viewDir, moonReflectDir), 0.0), 64.0);
    vec3 moonSpecular = moon.specular * moonSpec * moonAboveHorizon;

    vec3 specular = sunSpecular + moonSpecular;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

