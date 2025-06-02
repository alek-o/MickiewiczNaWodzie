#ifndef SINGLEMESH_H
#define SINGLEMESH_H

#include <GLAD/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

// This class purpose is to create, bind Vertex Arrays and Buffers, 
// set VertexAttribPointers and draw the meshes
class SingleMesh
{
private:
	float* vertices;
	GLuint* indices;
	GLuint VAO, VBO, EBO;
	GLsizei vertexCount, indexCount;
	GLsizei vertexParamsNumber, indexParamsNumber;

public:
	template <size_t N>
	SingleMesh(float (&vertices)[N], std::vector<GLuint> layout)
	{
		this->vertices = vertices;
		this->indices = nullptr;
		this->vertexCount = N;
		this->indexCount = NULL;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		GLuint currentOffset = 0;
		GLsizei stride = 0;
		for (GLuint i : layout) stride += i;
		vertexParamsNumber = stride;

		for (int i = 0; i < layout.size(); i++) {
			glVertexAttribPointer(i, layout[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(currentOffset * sizeof(float)));
			glEnableVertexAttribArray(i);
			currentOffset += layout[i];
		}

		glBindVertexArray(0);
	}
	template <size_t N, size_t M>
	SingleMesh(float (&vertices)[N], GLuint (&indices)[M], std::vector<GLuint> layout)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->vertexCount = N;
		this->indexCount = M;

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, EBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
		GLuint currentOffset = 0;
		GLsizei stride = 0;
		for (int i = 0; i < layout.size(); i++) {
			glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, layout[i], GL_FLOAT, GL_FALSE, stride * sizeof(GLuint), (void*)(currentOffset * sizeof(float)));
			currentOffset += layout[i];
		}

		glBindVertexArray(0);
	}

	void Draw() const {
		glBindVertexArray(VAO);
		if (indices == nullptr) {
			glDrawArrays(GL_TRIANGLES, 0, vertexCount / vertexParamsNumber);
		}
		else {
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);
	}

	~SingleMesh() {
		glDeleteBuffers(1, &VBO);
		if (indices != nullptr)
			glDeleteBuffers(1, &EBO);
		glDeleteVertexArrays(1, &VAO);
	}
};

#endif