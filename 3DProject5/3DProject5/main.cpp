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
int WIDTH = 900;
int HEIGHT = 600;
int RAYS_NUMBER = 100; // Dynamic ray count

// Background color
glm::vec3 background_color(0.1f, 0.1f, 0.1f); // Default dark gray
glm::vec3 ray_color(1.0f, 0.83f, 0.23f);

// Angle control variables
bool angle_mode = false; // Enable/Disable angle-based raycasting
float angle_range = 90.0f; // Angle range in degrees
float rotation_angle = 0.0f; // Rotation angle in degrees

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
    glm::vec2 direction;

    Ray(const glm::vec2& start, float angle)
        : start(start), direction(cos(angle), sin(angle)) {}
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

        float angle_step = glm::two_pi<float>() / RAYS_NUMBER;
        float start_angle = 0.0f;
        float end_angle = glm::two_pi<float>();

        if (angle_mode) {
            float half_range = glm::radians(angle_range / 2.0f);
            start_angle = glm::radians(rotation_angle) - half_range;
            end_angle = glm::radians(rotation_angle) + half_range;
            angle_step = (end_angle - start_angle) / RAYS_NUMBER;
        }

        for (int i = 0; i < RAYS_NUMBER; ++i) {
            float angle = start_angle + i * angle_step;
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
        // Ray direction
        float dx = ray.direction.x;
        float dy = ray.direction.y;

        // Calculate intersections with screen edges
        float t_min = std::numeric_limits<float>::max();

        // Top and bottom edges
        if (dy != 0) {
            float t_top = -ray.start.y / dy;
            float t_bottom = (HEIGHT - ray.start.y) / dy;
            t_min = std::min(t_min, std::max(t_top, t_bottom));
        }

        // Left and right edges
        if (dx != 0) {
            float t_left = -ray.start.x / dx;
            float t_right = (WIDTH - ray.start.x) / dx;
            t_min = std::min(t_min, std::max(t_left, t_right));
        }

        // If no intersection, skip this ray
        if (t_min < 0) return;

        // Compute endpoint on the screen boundary
        float end_x = ray.start.x + t_min * dx;
        float end_y = ray.start.y + t_min * dy;

        // Check collision with obstacle
        float obstacle_t = std::numeric_limits<float>::max();
        glm::vec2 obstacle_to_ray = ray.start - obstacle.position;

        float a = dx * dx + dy * dy;
        float b = 2.0f * (obstacle_to_ray.x * dx + obstacle_to_ray.y * dy);
        float c = glm::dot(obstacle_to_ray, obstacle_to_ray) - obstacle.radius * obstacle.radius;

        // Solve quadratic for intersection
        float discriminant = b * b - 4 * a * c;
        if (discriminant >= 0) {
            float sqrt_disc = sqrt(discriminant);
            float t1 = (-b - sqrt_disc) / (2.0f * a);
            float t2 = (-b + sqrt_disc) / (2.0f * a);

            if (t1 >= 0) obstacle_t = std::min(obstacle_t, t1);
            if (t2 >= 0) obstacle_t = std::min(obstacle_t, t2);
        }

        // Determine final endpoint (obstacle or screen)
        if (obstacle_t < t_min) {
            end_x = ray.start.x + obstacle_t * dx;
            end_y = ray.start.y + obstacle_t * dy;
        }

        // Draw ray
        glBegin(GL_LINES);
        glColor3f(ray_color.x,ray_color.y,ray_color.z); // Yellow color
        glVertex2f(ray.start.x, ray.start.y);
        glVertex2f(end_x, end_y);
        glEnd();
    }
};

// Callback for window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    float aspect_x = (float)width / WIDTH;
    float aspect_y = (float)height / HEIGHT;

    WIDTH = width;
    HEIGHT = height;

    // Adjust viewport
    glViewport(0, 0, width, height);

    // Adjust orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
    float obstacle_speed_x = 1.0f;

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    io.Fonts->AddFontDefault(); //ProggyClean

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Set background color
        glClearColor(background_color.r, background_color.g, background_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update obstacle position
        rayCaster.obstacle.position.y += obstacle_speed_y;
        rayCaster.obstacle.position.x += obstacle_speed_x;

        // Ensure obstacle bounces within bounds
        if (rayCaster.obstacle.position.y - rayCaster.obstacle.radius < 0 ||
            rayCaster.obstacle.position.y + rayCaster.obstacle.radius > HEIGHT) {
            obstacle_speed_y = -obstacle_speed_y;
        }

        if (rayCaster.obstacle.position.x - rayCaster.obstacle.radius < 0 ||
            rayCaster.obstacle.position.x + rayCaster.obstacle.radius > WIDTH) {
            obstacle_speed_x = -obstacle_speed_x;
        }

        // Poll for mouse input if not interacting with ImGui
        if (!ImGui::GetIO().WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            rayCaster.source.position = glm::vec2((float)xpos, (float)ypos);
            rayCaster.generateRays();
        }

        rayCaster.draw();
        rayCaster.generateRays();

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Dashboard");
        ImGui::Text("Controls");
        ImGui::Separator();

        
        ImGui::SliderFloat("Obstacle Speed Y", &obstacle_speed_y, -10.0f, 10.0f);
        ImGui::SliderFloat("Obstacle Speed X", &obstacle_speed_x, -10.0f, 10.0f);
        ImGui::SliderFloat("Source Radius", &rayCaster.source.radius, 5.0f, 100.0f);
        ImGui::SliderFloat("Obstacle Radius", &rayCaster.obstacle.radius, 5.0f, 100.0f);
        ImGui::SliderInt("Ray Count", &RAYS_NUMBER, 10, 1000);
        ImGui::Checkbox("Angle Mode", &angle_mode);
        if (angle_mode) {
            ImGui::SliderFloat("Angle Range", &angle_range, 10.0f, 360.0f);
            ImGui::SliderFloat("Rotation Angle", &rotation_angle, 0.0f, 360.0f);
        }
        ImGui::ColorEdit3("Ray Color", glm::value_ptr(ray_color));
        ImGui::ColorEdit3("Background Color", glm::value_ptr(background_color));
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
