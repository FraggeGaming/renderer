#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MouseController.h"
#include "Components/TransformComponent.h"

enum FPSDirection{
    LEFT, RIGHT, FORWARD, BACK, UP, DOWN
};

struct Plane{
    glm::vec3 normal;
    float d = 0.f; // Distance from origin
};

struct Frustum{
    Plane top;
    Plane bottom;
    Plane left;
    Plane right;
    Plane nearPlane;
    Plane farPlane;
};

struct Camera {
    
    MouseController mouse;

    glm::vec3 m_cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);


    glm::mat4 view = glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_up);

    Frustum frustum;

    glm::mat4 GetView(){
        view = glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_up);
        return view;
    }

    void CalculateFrustum(float aspectRatio, float fov, float nearDist, float farDist){
        const float halfVSide = farDist * tanf(glm::radians(fov) * 0.5f);
        const float halfHSide = halfVSide * aspectRatio;

        //Helpers
        glm::vec3 forward = glm::normalize(m_cameraFront);
        glm::vec3 right = glm::normalize(glm::cross(forward, m_up));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));

        glm::vec3 farCenter = m_cameraPosition + forward * farDist;
        glm::vec3 nearCenter = m_cameraPosition + forward * nearDist;

        frustum.nearPlane = { forward, -glm::dot(forward, nearCenter) };
        frustum.farPlane  = { -forward, -glm::dot(-forward, farCenter) };

        // far plane corners
        glm::vec3 fr = farCenter + right * halfHSide + up * halfVSide; // far-right-top
        glm::vec3 fl = farCenter - right * halfHSide + up * halfVSide; // far-left-top
        glm::vec3 br = farCenter + right * halfHSide - up * halfVSide; // far-right-bottom
        glm::vec3 bl = farCenter - right * halfHSide - up * halfVSide; // far-left-bottom

        auto makePlane = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c){
            glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));

            // Ensure normal points inward (towards frustum center)
            if (glm::dot(n, farCenter - a) < 0.0f) n = -n;
            float d = -glm::dot(n, a);
            return Plane{ n, d };
        };

        // camera position and two adjacent far-corners
        frustum.right  = makePlane(m_cameraPosition, fr, br);
        frustum.left   = makePlane(m_cameraPosition, bl, fl);
        frustum.top    = makePlane(m_cameraPosition, fl, fr);
        frustum.bottom = makePlane(m_cameraPosition, br, bl);
    }

    bool isVisible(const TransformComponent& transform) {
    // Calculate bounding box
    glm::vec3 min = transform.position - (transform.scale * 0.5f);
    glm::vec3 max = transform.position + (transform.scale * 0.5f);
    
    // Check each plane
    for (const Plane* plane : { &frustum.nearPlane, &frustum.farPlane, &frustum.left, &frustum.right, &frustum.top, &frustum.bottom }) {
        glm::vec3 positiveVertex = min;
        if (plane->normal.x >= 0) positiveVertex.x = max.x;
        if (plane->normal.y >= 0) positiveVertex.y = max.y;
        if (plane->normal.z >= 0) positiveVertex.z = max.z;

        
        if (glm::dot(plane->normal, positiveVertex) + plane->d < 0) {
            return false; //outside plane
        }
    }
    return true; //inside all planes
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

