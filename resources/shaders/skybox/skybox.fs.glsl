//#version 330 core
//out vec4 FragColor;

//in vec3 texCoords;

//uniform samplerCube skybox;

//void main()
//{
//	FragColor = texture(skybox, texCoords);
//}

#version 330 core

out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform vec3 sunColor;
uniform vec3 nightTint;
uniform float sunAltitude;

void main()
{
    vec3 texColor = texture(skybox, texCoords).rgb;

    texColor = texColor * 2.5 + vec3(0.8); // additive lift + scaling

    // Blend sun color into skybox
    float tintFactor = 0.6;
    vec3 dayColor = mix(texColor, sunColor, tintFactor);

    // Fade to night
    float dayToNight = clamp(sunAltitude, 0.0, 1.0);
    vec3 finalColor = mix(nightTint, dayColor, dayToNight);

    FragColor = vec4(finalColor, 1.0);
}

