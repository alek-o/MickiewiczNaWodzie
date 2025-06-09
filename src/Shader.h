#ifndef SHADER_H
#define SHADER_H

#include <GLAD/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* computePath, const char* vertexPath, const char* geometryPath, const char* fragmentPath)
    {
        // 1. retrieve the shader source code from filePath
        std::string computeCode;
        std::string vertexCode;
        std::string geometryCode;
        std::string fragmentCode;

        // Compute Shader
        if (computePath != nullptr) {
            std::ifstream cShaderFile;
            cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try {
                cShaderFile.open(computePath);
                std::stringstream cShaderStream;
                cShaderStream << cShaderFile.rdbuf();
                cShaderFile.close();
                computeCode = cShaderStream.str();
            }
            catch (std::ifstream::failure e) {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: COMPUTE" << std::endl;
            }
        }

        // Vertex Shader
        if (vertexPath != nullptr) {
            std::ifstream vShaderFile;
            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try {
                vShaderFile.open(vertexPath);
                std::stringstream vShaderStream;
                vShaderStream << vShaderFile.rdbuf();
                vShaderFile.close();
                vertexCode = vShaderStream.str();
            }
            catch (std::ifstream::failure e) {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: VERTEX" << std::endl;
            }
        }

        // Geometry Shader
        if (geometryPath != nullptr) {
            std::ifstream gShaderFile;
            gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
            catch (std::ifstream::failure e) {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: GEOMETRY" << std::endl;
            }
        }

        // Fragment Shader
        if (fragmentPath != nullptr) {
            std::ifstream fShaderFile;
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try {
                fShaderFile.open(fragmentPath);
                std::stringstream fShaderStream;
                fShaderStream << fShaderFile.rdbuf();
                fShaderFile.close();
                fragmentCode = fShaderStream.str();
            }
            catch (std::ifstream::failure e) {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: FRAGMENT" << std::endl;
            }
        }

        // 2. compile shaders
        unsigned int compute = 0, vertex = 0, geometry = 0, fragment = 0;

        // Compute Shader
        if (computePath != nullptr) {
            const char* cShaderCode = computeCode.c_str();
            compute = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(compute, 1, &cShaderCode, NULL);
            glCompileShader(compute);
            checkCompileErrors(compute, "COMPUTE");
        }

        // Vertex Shader
        if (vertexPath != nullptr) {
            const char* vShaderCode = vertexCode.c_str();
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");
        }

        // Geometry Shader
        if (geometryPath != nullptr) {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }

        // Fragment Shader
        if (fragmentPath != nullptr) {
            const char* fShaderCode = fragmentCode.c_str();
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
        }

        // Shader Program
        ID = glCreateProgram();
        if (computePath != nullptr) glAttachShader(ID, compute);
        if (vertexPath != nullptr) glAttachShader(ID, vertex);
        if (geometryPath != nullptr) glAttachShader(ID, geometry);
        if (fragmentPath != nullptr) glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the shaders as they're linked into our program now and no longer necessary
        if (computePath != nullptr) glDeleteShader(compute);
        if (vertexPath != nullptr) glDeleteShader(vertex);
        if (geometryPath != nullptr) glDeleteShader(geometry);
        if (fragmentPath != nullptr) glDeleteShader(fragment);
    }
    // use/activate the shader
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setUInt(const std::string& name, int value) const
    {
        glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setMat4(const std::string& name, const glm::mat4& value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif
