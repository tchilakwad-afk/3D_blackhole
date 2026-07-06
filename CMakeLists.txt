cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/glad.c")

add_executable(${PROJECT_NAME} ${SOURCES}
    src/main.cpp)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders/"
    TEXTURE_DIR="${CMAKE_SOURCE_DIR}/resources/textures/"
    MODEL_DIR="${CMAKE_SOURCE_DIR}/resources/models/"
)
target_include_directories(${PROJECT_NAME} PRIVATE
    include/   
)

find_package(glm REQUIRED)
find_package(assimp REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    assimp::assimp
)