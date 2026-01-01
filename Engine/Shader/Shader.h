#include <fstream>
#include <GL/glew.h>

#pragma once

struct ShaderSrc {
    std::string VertexSrc;
    std::string FragmentSrc;
};

class Shader{
private:
    unsigned int ID;
    bool isCompiled = false;


    

public:
    Shader(const std::string& vertexShader, const std::string& fragmentShader);
    ~Shader();

    void Use();
    void SetFloat(const std::string &name, float value) const;
    void SetInt(const std::string &name, int value) const;
    void SetBool(const std::string &name, bool value) const;
    void SetMat4(const std::string &name, const float* value, GLboolean transpose ) const;
    void SetVec3(const std::string &name, const float* value) const;
    void SetVec2(const std::string &name, const float* value) const;
    void SetVec4(const std::string &name, const float* value) const;
    bool IsCompiled(){return isCompiled;}


    static unsigned int CompileShader(unsigned int type, const std::string& source);
    static ShaderSrc ParseShader(const std::string& path);
};