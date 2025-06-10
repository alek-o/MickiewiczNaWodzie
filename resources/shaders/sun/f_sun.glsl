//#version 430 core
//out vec4 FragColor;

//in vec3 ourColor;

//void main()
//{
//    FragColor = vec4(ourColor, 1.0);
//}

#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sunTexture;

void main()
{
    FragColor = texture(sunTexture, TexCoord);
}