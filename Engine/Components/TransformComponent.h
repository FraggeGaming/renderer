#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma once

struct TransformComponent{
    
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::mat4 m_transform = glm::mat4(1.0f);

    bool isDirty = false;


    void SetPosition(glm::vec3 vec){
        position = glm::vec3(vec);
        isDirty = true;
    }

    void SetRotation(glm::vec3 eulerAngles){
        rotation = glm::quat(eulerAngles);    
        isDirty = true;

    }

    void AddRotation(glm::vec3 eulerAngles){
        rotation *= glm::quat(eulerAngles);
        glm::quat delta = glm::quat(glm::radians(eulerAngles));
        rotation = rotation * delta;
        rotation = glm::normalize(rotation);
        isDirty = true;

    }

    void Translate(float dx, float dy, float dz){
        position += glm::vec3(dx, dy, dz);
        isDirty = true;

    }

    void Scale(float x, float y, float z){
        scale *= glm::vec3(x, y, z);
        isDirty = true;

    }


    glm::mat4 GetCombined() const{

        glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationMat = glm::toMat4(rotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);


        return translationMat * rotationMat * scaleMat;
    }
    
};