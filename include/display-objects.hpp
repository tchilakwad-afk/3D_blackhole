#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Camera{
    public: 
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    bool isDragging;
    double mousePosX;
    double mousePosY;


    Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, float cam_fov_degrees, float cam_aspect_ratio, float cam_min_view_distance, float cam_max_view_distance);
    void move(glm::vec3 cameraMovementVector);
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    void handleMouseScroll(float yOffset);
    void handleMouseMove(float xOffset, float yOffset);
    // void turn(glm::vec3 cameraTurnVec; float anglesInDegrees);
    private:
    glm::mat4 viewMat;
    glm::mat4 projMat;

    float fov_deg;
    float aspect_ratio;
    float min_view_distance;
    float max_view_distance;
};

