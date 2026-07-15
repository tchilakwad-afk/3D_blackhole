#pragma once 

constexpr unsigned int SCREEN_WIDTH = 800;
constexpr unsigned int SCREEN_HEIGHT = 600;

const unsigned int TEXTURE_WIDTH = 500;
const unsigned int TEXTURE_HEIGHT = 500;

constexpr float PI = 3.14159265358979323846f;
constexpr float SCROLL_SENSITIVITY = 0.1f;
constexpr float ROTATION_SENSITIVITY = 0.0015f;

constexpr float MAX_DS = 10.0f;
constexpr float KEYBOARD_MOVE_SPEED = 15.0f;

inline float clamp(float x, float min, float max){
    return x<min?min: x>max?max:x;
}

constexpr unsigned int MAX_PARTICLES = 256*256;
constexpr float DT_FACTOR  = 1.9f;