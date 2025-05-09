#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <cmath>
#include <chrono>

#define M_PI 3.14159265358979323846

struct Coords {
	double x, y, z;
};

void error_callback(int error, const char* description) {
	std::cerr << "Error: " << description << std::endl;
}

double GetMToLight(double x, double y, double z, Coords Camera) {
	double tanH = (x - Camera.x) / (z - Camera.z);
	double tanV = (y - Camera.y) / (z - Camera.z);
	double R = 1;
	double a = pow(tanH, 2) + pow(tanV, 2) + 1;
	double b = -2 * tanH * tanH * Camera.z - 2 * tanV * tanV * Camera.z + 2 * tanH * Camera.x + 2 * tanV * Camera.y;
	double c = tanH * Camera.z * (tanH * Camera.z - 2 * Camera.x) + tanV * Camera.z * (tanV * Camera.z - 2 * Camera.y) + pow(Camera.x, 2) + pow(Camera.y, 2) - pow(R, 2);
	double Discriminant = pow(b, 2) - 4 * a * c;
	if (Discriminant < 0) {
		return 1e10;
	}
	else {
		double z1 = (-b + sqrt(Discriminant)) / (2 * a);
		double z2 = (-b - sqrt(Discriminant)) / (2 * a);
		double x1 = tanH * z1 - tanH * Camera.z + Camera.x;
		double y1 = tanV * z1 - tanV * Camera.z + Camera.y;
		x1 = x1 - Camera.x;
		y1 = y1 - Camera.y;
		z1 = z1 - Camera.z;
		double x2 = tanH * z2 - tanH * Camera.z + Camera.x;
		double y2 = tanV * z2 - tanV * Camera.z + Camera.y;
		x2 = x2 - Camera.x;
		y2 = y2 - Camera.y;
		z2 = z2 - Camera.z;
		double m1 = sqrt(pow(x1, 2) + pow(y1, 2) + pow(z1, 2));
		double m2 = sqrt(pow(x2, 2) + pow(y2, 2) + pow(z2, 2));
		double m = std::min(m1, m2);
		//std::cout << m << ' ';
		if (m == m1 && std::round(z1 * 1e4) / 1e4 == std::round(z * 1e4) / 1e4 || m == m2 && std::round(z2 * 1e4) / 1e4 == std::round(z * 1e4) / 1e4) return m;
		else return 1e10;

	}
	return 0;
}

int main() {

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwSetErrorCallback(error_callback);

	int width = 640;
	int height = 480;
	GLFWwindow* window = glfwCreateWindow(width, height, "REAL TRI D", NULL, NULL);
	// glfwMaximizeWindow(window);

	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	glPointSize(1.0f); // Размер точки — 1 пиксель


	Coords Camera = { 0.0, 0.0, -2.5 };

	Coords Light = { 5.0, 5.0, 0.0 };

	long long TimeFPS = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
	long long FPS = 0;

	while (!glfwWindowShouldClose(window)) {

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		//glDrawArrays(GL_POINTS, 0, 1);
		glBegin(GL_POINTS);

		for (int y = height / -2; y <= height / 2; y++) {
			for (int x = width / -2; x <= width / 2; x++) {
				double tanH = tan(x / (width / 2.0) * 60.0 / 180.0 * M_PI);
				double tanV = tan(y / (height / 2.0) * 45.0 / 180.0 * M_PI);
				double R = 1;
				double a = pow(tanH, 2) + pow(tanV, 2) + 1;
				double b = -2 * tanH * tanH * Camera.z - 2 * tanV * tanV * Camera.z + 2 * tanH * Camera.x + 2 * tanV * Camera.y;
				double c = tanH * Camera.z * (tanH * Camera.z - 2 * Camera.x) + tanV * Camera.z * (tanV * Camera.z - 2 * Camera.y) + pow(Camera.x, 2) + pow(Camera.y, 2) - pow(R, 2);
				double Discriminant = pow(b, 2) - 4 * a * c;
				if (Discriminant < 0) {
					glColor3f(0.0f, 0.0f, 0.0f);
				}
				else {
					double z1 = (-b + sqrt(Discriminant)) / (2 * a);
					double z2 = (-b - sqrt(Discriminant)) / (2 * a);
					double x1 = tanH * z1 - tanH * Camera.z + Camera.x;
					double y1 = tanV * z1 - tanV * Camera.z + Camera.y;
					x1 = x1 - Camera.x;
					y1 = y1 - Camera.y;
					z1 = z1 - Camera.z;
					double x2 = tanH * z2 - tanH * Camera.z + Camera.x;
					double y2 = tanV * z2 - tanV * Camera.z + Camera.y;
					x2 = x2 - Camera.x;
					y2 = y2 - Camera.y;
					z2 = z2 - Camera.z;
					double m1 = sqrt(pow(x1, 2) + pow(y1, 2) + pow(z1, 2));
					double m2 = sqrt(pow(x2, 2) + pow(y2, 2) + pow(z2, 2));
					double m = std::min(m1, m2);
					if (m == m1) {
						x1 = x1 + Camera.x;
						y1 = y1 + Camera.y;
						z1 = z1 + Camera.z;
						m = GetMToLight(x1, y1, z1, Light);
					}
					else {
						x2 = x2 + Camera.x;
						y2 = y2 + Camera.y;
						z2 = z2 + Camera.z;
						m = GetMToLight(x2, y2, z2, Light);
					}
					// std::cout << m << ' ';
					glColor3f(1.0f / m, 1.0f / m, 1.0f / m);
				}
				glVertex2f(x / (width / 2.0), y / (height / 2.0));
			}
		}
		glEnd();

		if (std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() - TimeFPS < 1000) FPS++;
		else {
			std::cout << "fps: " << FPS << '\n';
			FPS = 0;
			TimeFPS = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			).count();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}