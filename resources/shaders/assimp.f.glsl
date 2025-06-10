//#version 330 core
//out vec4 FragColor;  

//in vec2 TexCoord;

//uniform sampler2D texture_diffuse1;

//void main()
//{
//	FragColor = texture(texture_diffuse1, TexCoord);
//}

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
uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-sun.direction);

    float aboveHorizon = dot(-sun.direction, vec3(0.0, 1.0, 0.0)) > -0.05 ? 1.0 : 0.0;

    // Ambient
    vec3 ambient = sun.ambient * texture(texture_diffuse1, TexCoord).rgb * aboveHorizon;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = sun.diffuse * diff * texture(texture_diffuse1, TexCoord).rgb * aboveHorizon;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = sun.specular * spec * aboveHorizon;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

