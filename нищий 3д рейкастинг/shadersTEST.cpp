// main.cpp
// Пример переноса вычитаний пересечения сферы и затенения на GPU

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

// Шейдеры
const char* vertexShaderSrc = R"(
#version 330 core
out vec2 uv;
void main() {
    uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vec2 pos = uv * 2.0 - 1.0;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
in vec2 uv;
out vec4 FragColor;
uniform vec3 Camera;
uniform vec3 Light;
uniform vec2 iResolution;
const float R = 1.0;

// epsilon to avoid self-intersection
const float EPS = 0.0001;

void main() {
    vec2 p = uv * 2.0 - 1.0;
    float tanH = tan(p.x * radians(60.0));
    float tanV = tan(p.y * radians(45.0));
    vec3 dir = normalize(vec3(tanH, tanV, 1.0));

    // primary ray-sphere intersection
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, Camera);
    float c = dot(Camera, Camera) - R * R;
    float D = b*b - 4.0 * a * c;
    if (D < 0.0) { FragColor = vec4(0.0); return; }
    float sqrtD = sqrt(D);
    float t1 = (-b + sqrtD) / (2.0 * a);
    float t2 = (-b - sqrtD) / (2.0 * a);
    float t = min(t1, t2);
    if (t < EPS) {
        t = max(t1, t2);
        if (t < EPS) { FragColor = vec4(0.0); return; }
    }

    vec3 P = Camera + dir * t;

    // shadow ray: from P toward Light
    vec3 Ldir = normalize(Light - P);
    float lightDist = length(Light - P);
    // sphere intersection along shadow ray
    float bd = 2.0 * dot(Ldir, P);
    float cd = dot(P, P) - R * R;
    float Dd = bd*bd - 4.0 * cd;
    bool inShadow = false;
    if (Dd > 0.0) {
        float sd1 = (-bd + sqrt(Dd)) * 0.5;
        float sd2 = (-bd - sqrt(Dd)) * 0.5;
        float sd = sd1 > EPS ? sd1 : (sd2 > EPS ? sd2 : -1.0);
        if (sd > EPS && sd < lightDist) inShadow = true;
    }

    float intensity = inShadow ? 0.0 : 1.0 / lightDist;
    FragColor = vec4(vec3(intensity), 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return s;
}

GLuint createProgram(const char* vs, const char* fs) {
    GLuint vsId = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fsId = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsId);
    glAttachShader(prog, fsId);
    glLinkProgram(prog);
    glDeleteShader(vsId);
    glDeleteShader(fsId);
    return prog;
}

int main() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return -1;
    int width = 640, height = 480;
    GLFWwindow* window = glfwCreateWindow(width, height, "REAL TRI D", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return -1;
    glPointSize(1.0f);

    // Camera, Light и тайминги
    Coords Camera = { 0.0, 0.0, -2.5 };
    Coords Light  = { 5.0, 3.0, 0.0 };

    auto TimeLast = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    auto TimeFPS = TimeLast;
    auto TimeControlFPS = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    int FPS = 0;

    GLuint shaderProgram = createProgram(vertexShaderSrc, fragmentShaderSrc);
    GLuint vao; glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    GLint uniCamera = glGetUniformLocation(shaderProgram, "Camera");
    GLint uniLight  = glGetUniformLocation(shaderProgram, "Light");
    GLint uniRes    = glGetUniformLocation(shaderProgram, "iResolution");

    while (!glfwWindowShouldClose(window)) {
        auto TimeNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        double dt = (TimeNow - TimeLast) / 1000.0;
        TimeLast = TimeNow;

        // управление камерой (твоим кодом)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) Camera.z += dt * 3;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) Camera.z -= dt * 3;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) Camera.x -= dt * 3;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) Camera.x += dt * 3;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)     Camera.y += dt * 3;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)Camera.y -= dt * 3;

        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform3f(uniCamera, Camera.x, Camera.y, Camera.z);
        glUniform3f(uniLight,  Light.x,  Light.y,  Light.z);
        glUniform2f(uniRes,    (float)w, (float)h);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        

        while (TimeControlFPS - TimeLast < 1000.0 / 60.0) {
            TimeControlFPS = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }
        if (TimeNow - TimeFPS < 1000) FPS++;
        else {
            std::cout << "fps: " << FPS << "\n";
            FPS = 0;
            TimeFPS = TimeNow;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
