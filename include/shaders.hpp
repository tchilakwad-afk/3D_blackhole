#pragma once 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "display-objects.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ShaderBase{
    public:
        unsigned int ID; 
        void use();
        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec2(const std::string &name, glm::vec2 value) const;
        void setVec3(const std::string &name, glm::vec3 value) const;
        void setMat4(const std::string &name, glm::mat4 mat) const;
        void checkCompileErrors(unsigned int shader, std::string type);
        void setUint(const std::string &name, int unsigned value) const;
        GLint getShaderUniformLocation(const std::string& name) const;
    private:
        mutable std::unordered_map<std::string, GLint> uniforms_loaded;
};

class GraphicsShaders: public ShaderBase{
    public:
        GraphicsShaders(const std::string vertexShaderFilePath, const std::string fragmentShaderFilePath);
};

class ComputeShaders:public ShaderBase{
    public:
        ComputeShaders(const std::string computeShaderFilePath);
        void setExecutionParameters(unsigned int workgroup_sized[3], unsigned int memBarrier);
        void execute();
        
    private:
        unsigned int workgroup_sizes[3];
        unsigned int memBarrier;
    
};