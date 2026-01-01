#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#pragma once

struct Material{
    int ID;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;


    bool operator==(const Material& other) const {
        
        return ID == other.ID;
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCord;
    glm::vec3 normal;
    glm::vec4 color;
};

struct Mesh{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};