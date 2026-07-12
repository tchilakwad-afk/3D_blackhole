
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "aux.hpp"
#include "display-objects.hpp"
#include "constants.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;


void processInput(GLFWwindow *window, Camera& camera, float ds)
{

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    glm::vec3 movementVec({0.0f, 0.0f, 0.0f});
    const bool W = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    const bool A = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    const bool S = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    const bool D = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    if(W) movementVec+= camera.front;
    if(A) movementVec-= camera.right;
    if(S) movementVec-= camera.front;
    if(D) movementVec+= camera.right;
    if(W|A|S|D) camera.move(movementVec*ds);   
}

// void mouse_scroll_callback(GLFWwindow* window, double offsetX, double offsetY){
//     Camera* cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
//     // cam->
// }

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Camera* cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            cam->isDragging = true;
            double lastX, lastY;
            glfwGetCursorPos(window, &lastX, &lastY);
            cam->mousePosX = lastX;
            cam->mousePosY = lastY;
        } else if (action == GLFW_RELEASE) {
            cam->isDragging = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Camera* cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (!cam->isDragging) return;

    double dx = clamp(-(xpos - cam->mousePosX), -MAX_DS, MAX_DS);
    double dy = clamp((ypos - cam->mousePosY), -MAX_DS, MAX_DS);
    
    cam->mousePosX = xpos;
    cam->mousePosY = ypos;
    cam->handleMouseMove(dx, dy);
}
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    Camera* cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    cam->handleMouseScroll(yoffset);
}

GLFWwindow* setupWindow(Camera &cam, int width, int height){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL practice", NULL, NULL);
    glfwSetWindowUserPointer(window, &cam);
    if(window==NULL){
        std::cout<<"Failed to create GLFW window" <<std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);



    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout<<"Failed to initialize glad"<<std::endl;
        exit(1);
    }
    return window;
}
std::string readSourceFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader: " + filepath);

    std::stringstream ss;
    ss << file.rdbuf();

    if (ss.fail())
        throw std::runtime_error("Failed to read shader: " + filepath);

    if (ss.str().empty())
        throw std::runtime_error("Shader file is empty: " + filepath);

    return ss.str();
}



unsigned int readTextureFromFile(std::string filepath, int& width, int& height, int& nChannels){
    unsigned int texture;
    // glGenTextures(1, &texture);
    // glBindTexture(GL_TEXTURE_2D, texture);
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    if(filepath==""){
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTextureStorage2D(texture, 1, GL_RGB8, width, height);
        return texture;
    }
    std::cout<<"Loading texture...\n ";
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nChannels, 0);
    if (data)
    {
        
        // std::cout << "Texture 1 " << nChannels << std::endl;
        GLenum format = (nChannels == 4) ? GL_RGBA : GL_RGB;
        GLenum internalFormat = (nChannels == 4) ? GL_RGBA8: GL_RGB8;
        int mipLevels = (int)std::floor(std::log2(std::max(width, height)));
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // std::cout << "Unpacking alignment done\n ";
        glTextureStorage2D(texture, mipLevels, internalFormat, width, height);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
        // glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(texture);
    }
    else
    {
        std::cout << "ERROR::TEXTURE_LOAD::FILEPATH:"<<filepath<< std::endl;
    }
    std::cout <<"Freeing data...\n ";
    stbi_image_free(data);
    return texture;

}

unsigned int loadCubemap(vector<string> faces){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nChannels;
    for(unsigned int i = 0; i < faces.size(); i++){
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nChannels, 0);
        if(data){
            GLenum format = (nChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else{
            cout << "ERROR :: CUBEMAP_LOAD :: FILEPATH:" << faces[i] << endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}