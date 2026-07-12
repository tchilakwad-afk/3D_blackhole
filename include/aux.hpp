#pragma once 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <random>
#include <vector>

#include "display-objects.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera& camera, float deltaTime);
// void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
inline float randFloat(const glm::vec2& bounds){
    return bounds.x + ((rand()*1.0f)/(RAND_MAX))*(bounds.y - bounds.x);
}
unsigned int loadCubemap(std::vector<std::string> faces);

GLFWwindow* setupWindow(Camera &cam, int width, int height);
std::string readSourceFromFile(const std::string& filepath);
unsigned int readTextureFromFile(std::string filepath, int& width, int& height, int& nChannels);