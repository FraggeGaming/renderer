#include "Camera.h"
#include <iostream>





void Camera::CalculateFrustum(float aspectRatio, float fov, float nearDist, float farDist){
    const float halfVSide = farDist * tanf(glm::radians(fov) * 0.5f);
    const float halfHSide = halfVSide * aspectRatio;
    const glm::vec3 frontMultFar = m_cameraFront * farDist;

    glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_up));
    glm::vec3 up = glm::normalize(glm::cross(right, m_cameraFront));
    

    //Far
    frustum.planes[0] = CreatePlaneFrustum(-m_cameraFront, m_cameraPosition + (frontMultFar) );
    //Near
    frustum.planes[1] = CreatePlaneFrustum(m_cameraFront, m_cameraPosition + (nearDist * m_cameraFront));

    //Right 
    frustum.planes[2] = CreatePlaneFrustum(glm::cross(frontMultFar - right * halfHSide, up), m_cameraPosition);

    //Left
    frustum.planes[3] = CreatePlaneFrustum(glm::cross(up, frontMultFar + right * halfHSide), m_cameraPosition);
    //Top
    frustum.planes[4] = CreatePlaneFrustum(glm::cross(right, frontMultFar - up * halfVSide), m_cameraPosition);
    //Bottom
    frustum.planes[5] = CreatePlaneFrustum(glm::cross(frontMultFar + up * halfVSide, right), m_cameraPosition);
    
}

bool Camera::isVisible(const TransformComponent& transform) {

    //Sphere-Frustum Culling
    for(size_t i = 0; i < 6; i++) {
        float radius = (transform.scale.x + transform.scale.y) / 2.0f; // Approximate radius
        const Plane& plane = frustum.planes[i];
        float dist = glm::dot(transform.position, plane.normal) + plane.d + radius;

        if (dist <= 0) {
            //std::cout << "Rejected by plane " << i << " Dist: " << dist << std::endl;
            return false; 
        }
    }

    //std::cout << "Entity at " << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << " is visible." << std::endl;
    
    return true; //inside all planes
}



