#pragma once

#include <glad/glad.h>
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
using namespace glm;

#include <vector>
#include <cmath>

using namespace std;

#define PI 3.14159265358979323846

double c = 299792458.0;
double G = 6.67430e-11;
double prog_scale = 1e8;
double c_pixels = c / prog_scale;

struct Ray;
struct BlackHole;
void geodesic(double& d2t, double& d2r, double& d2phi, Ray ray, BlackHole SagA);
void runge_kutta4(double dlambda, Ray& ray, BlackHole SagA);

struct BlackHole{
	vec3 position;
	double mass;
	double r_s;

	BlackHole(vec3 pos, float m);

	void draw(GLFWwindow& window);
};

// struct Ray{
// 	vec3 position;
// 	vec3 velocity;
// 	// vector<Vertex> trail;
// 	float currentTime = 0.0f;
// 	bool active;

// 	double r; double phi; double theta; double t;
// 	double dr; double dphi; double dtheta; double dt;

// 	Ray(vec3 pos, vec3 vel, BlackHole SagA);

// 	void update(float dlambda, BlackHole SagA);

// 	void draw(GLFWwindow& window);
// };