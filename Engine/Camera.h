#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MouseController.h"

enum FPSDirection{
    LEFT, RIGHT, FORWARD, BACK, UP, DOWN
};

struct Camera {
    
    MouseController mouse;

    glm::vec3 m_cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);


    glm::mat4 view = glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_up);

    glm::mat4 GetView(){
        view = glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_up);
        return view;
    }

    void MoveFPS(float dy, FPSDirection direction){
        switch (direction){
            case FPSDirection::LEFT:
                m_cameraPosition -= dy * glm::normalize(glm::cross(m_cameraFront, m_up));
                break;
            case FPSDirection::RIGHT:
                m_cameraPosition += dy * glm::normalize(glm::cross(m_cameraFront, m_up));
                break;
            case FPSDirection::FORWARD:
                m_cameraPosition += dy * m_cameraFront;
                break;
            case FPSDirection::BACK:
                m_cameraPosition -= dy * m_cameraFront;
                break;
            case FPSDirection::UP:
                m_cameraPosition -= dy * -m_up;
                break;
            case FPSDirection::DOWN:
                m_cameraPosition -= dy * m_up;
                break;
        }
    }

};

