#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"display-objects.hpp"
#include "constants.hpp"

Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, float cam_fov_degrees, float cam_aspect_ratio, float cam_min_view_distance, float cam_max_view_distance){
    pos=  cameraPos;
    front = cameraFront;
    up = cameraUp;
    right = glm::cross(front, up);
    viewMat = glm::lookAt(pos, pos+front, up);
    fov_deg = cam_fov_degrees;
    aspect_ratio = cam_aspect_ratio;
    min_view_distance = cam_min_view_distance;
    max_view_distance = cam_max_view_distance;
    projMat = glm::perspective(glm::radians(fov_deg), aspect_ratio, min_view_distance, max_view_distance);
}

glm::mat4 Camera::getViewMatrix(){
    return viewMat;
}
glm::mat4 Camera::getProjectionMatrix(){
    return projMat;
}
void Camera::handleMouseScroll(float yOffset){
    pos += up*SCROLL_SENSITIVITY*yOffset;
    viewMat = glm::lookAt(pos, pos+front, up);
}

void Camera::handleMouseMove(float xOffset, float yOffset){
    if(xOffset*xOffset + yOffset*yOffset <= 1e-3) return;
    glm::vec3 offsetVec = glm::normalize(right*xOffset + up*yOffset);
    glm::vec3 rotationVec = glm::cross(front, offsetVec);
    float rotationValue = sqrtf(xOffset*xOffset + yOffset*yOffset) * ROTATION_SENSITIVITY;
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, rotationValue, rotationVec);
    up = glm::vec3(rotMat* glm::vec4(up, 0.0f));
    right = glm::vec3(rotMat* glm::vec4(right, 0.0f));
    front = glm::vec3(rotMat* glm::vec4(front, 0.0f));
    
    // right *= rotMat;
    // front *= rotMat;
    viewMat = glm::lookAt(pos, pos+front, up);
}


void Camera::move(glm::vec3 cameraMovementVector){
    pos += cameraMovementVector;
    viewMat = glm::lookAt(pos, pos+front, up);
}
