#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


Shader::Shader(const std::string &vertexShader, const std::string &fragmentShader)
{
    ID = glCreateProgram();
    //std::cout << "Vertex Shader:\n"
      //        << vertexShader << std::endl;
    //std::cout << "Fragment Shader:\n"
      //        << fragmentShader << std::endl;

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);
    glValidateProgram(ID);

    glDeleteShader(vs);
    glDeleteShader(fs);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLchar info[512];
        glGetProgramInfoLog(ID, 512, NULL, info);
        std::cout << info << std::endl;
    }

    else{
        isCompiled = true;
    }
}

ShaderSrc Shader::ParseShader(const std::string &path)
{
    std::ifstream stream(path);
    std::string line;

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        // std::cout << line << std::endl;
        if (line.find("#shader") != std::string::npos)
        { // has found the string
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }

        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return {ss[0].str(), ss[1].str()};
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

void Shader::Use()
{
    glUseProgram(ID);
}

void Shader::SetFloat(const std::string &name, float value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniform1f(location, value);
}

void Shader::SetInt(const std::string &name, int value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniform1i(location, value);
}
void Shader::SetBool(const std::string &name, bool value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniform1i(location, (int)value);
}
void Shader::SetMat4(const std::string &name, const float *value, GLboolean transpose) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniformMatrix4fv(location, 1, transpose, value);
}


void Shader::SetVec3(const std::string &name, const float *value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniform3fv(location, 1, value);
}
void Shader::SetVec2(const std::string &name, const float *value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniform2fv(location, 1, value);
}
void Shader::SetVec4(const std::string &name, const float *value) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cout << "[Shader Warning] Uniform '" << name << "' not found.\n";
        return;
    }
    glUniform4fv(location, 1, value);
}

unsigned int Shader::CompileShader(unsigned int type, const std::string &source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar info[512];
        glGetShaderInfoLog(id, 512, nullptr, info);
        std::cout << "Shader compilation failed:\n"
                  << info << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}