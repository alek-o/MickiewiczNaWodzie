#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <string>

class Texture2D
{
private:
	GLuint textureID;
	std::string texPath;
	int texUnitIndex;
public:

	//Texture2D(GLuint textureID, std::string texPath, GLenum texSlot)
	Texture2D(std::string texPath, int texUnitIndex)
	{
		this->texPath = texPath;
		this->texUnitIndex = texUnitIndex;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(texPath.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}

	void bind()
	{
		glActiveTexture(GL_TEXTURE0 + texUnitIndex);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	GLuint getSlot()
	{
		return  texUnitIndex;
	}
};
#endif