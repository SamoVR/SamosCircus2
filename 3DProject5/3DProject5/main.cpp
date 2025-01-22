// Standard Libraries and OpenGL Includes
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLM_ENABLE_EXPERIMENTAL

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>

// Constants
constexpr int WIDTH = 900;
constexpr int HEIGHT = 600;
constexpr int RAYS_NUMBER = 100;

// Circle Class
class Circle {
public:
    glm::vec2 position;
    float radius;

    Circle(float x, float y, float r) : position(x, y), radius(r) {}

    void draw(const glm::vec3& color) const {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(color.r, color.g, color.b);
        glVertex2f(position.x, position.y); // Center
        for (int i = 0; i <= 360; ++i) {
            float angle = glm::radians(static_cast<float>(i));
            float x = position.x + radius * cos(angle);
            float y = position.y + radius * sin(angle);
            glVertex2f(x, y);
        }
        glEnd();
    }
};

// Ray Class
class Ray {
public:
    glm::vec2 start;
    float angle;

    Ray(const glm::vec2& start, float angle) : start(start), angle(angle) {}
};

// RayCaster Class
class RayCaster {
public:
    Circle source;
    Circle obstacle;
    std::vector<Ray> rays;

    RayCaster(float source_x, float source_y, float source_r, float obstacle_x, float obstacle_y, float obstacle_r)
        : source(source_x, source_y, source_r), obstacle(obstacle_x, obstacle_y, obstacle_r) {
        generateRays();
    }

    void generateRays() {
        rays.clear();
        for (int i = 0; i < RAYS_NUMBER; ++i) {
            float angle = ((float)i / RAYS_NUMBER) * glm::two_pi<float>();
            rays.emplace_back(source.position, angle);
        }
    }

    void draw() {
        source.draw(glm::vec3(1.0f, 1.0f, 1.0f)); // Draw source (white)
        obstacle.draw(glm::vec3(1.0f, 0.0f, 0.0f)); // Draw obstacle (red)

        for (const auto& ray : rays) {
            drawRay(ray);
        }
    }

private:
    void drawRay(const Ray& ray) {
        float x = ray.start.x;
        float y = ray.start.y;
        float step = 1.0f;
        bool end_of_screen = false;
        bool hit_object = false;

        glBegin(GL_LINES);
        glColor3f(1.0f, 0.83f, 0.23f); // Yellow color
        glVertex2f(x, y);

        while (!end_of_screen && !hit_object) {
            x += step * cos(ray.angle);
            y += step * sin(ray.angle);

            // Check bounds
            if (x < 0 || x > WIDTH || y < 0 || y > HEIGHT) {
                end_of_screen = true;
            }

            // Check collision with obstacle
            glm::vec2 point(x, y);
            float dist_squared = glm::dot(point - obstacle.position, point - obstacle.position);
            if (dist_squared <= obstacle.radius * obstacle.radius) {
                hit_object = true;
            }
        }
        glVertex2f(x, y);
        glEnd();
    }
};

// Main Function
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a GLFW Window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ray Casting Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Set up OpenGL
    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Create RayCaster Object
    RayCaster rayCaster(WIDTH / 2.0f, HEIGHT / 2.0f, 50.0f, WIDTH / 2.0f, HEIGHT / 4.0f, 30.0f);
    float obstacle_speed_y = 2.0f;

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Update obstacle position
        rayCaster.obstacle.position.y += obstacle_speed_y;
        if (rayCaster.obstacle.position.y - rayCaster.obstacle.radius < 0 ||
            rayCaster.obstacle.position.y + rayCaster.obstacle.radius > HEIGHT) {
            obstacle_speed_y = -obstacle_speed_y;
        }

        // Poll for mouse input
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            rayCaster.source.position = glm::vec2((float)xpos, (float)ypos);
            rayCaster.generateRays();
        }

        // Draw RayCaster
        rayCaster.draw();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui UI
        ImGui::Begin("Controls");
        ImGui::Text("Adjust Obstacle Speed:");
        ImGui::SliderFloat("Speed", &obstacle_speed_y, -10.0f, 10.0f);
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
