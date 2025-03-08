//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

const char* vertexShaderSource = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float lightIntensity;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
    vec3 normal = normalize(aPos);
    vec3 dirToCenter = normalize(-worldPos);
    lightIntensity = max(dot(normal, dirToCenter), 0.15);})glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
in float lightIntensity;
out vec4 FragColor;
uniform vec4 objectColor;
uniform bool isGrid; // Add this uniform
uniform bool GLOW;
void main() {
    if (isGrid) {
        // If it's the grid, use the original color without lighting
        FragColor = objectColor;
    } else if(GLOW){
        FragColor = vec4(objectColor.rgb * 100000, objectColor.a);
    }else {
        // If it's an object, apply the lighting effect
        float fade = smoothstep(0.0, 10.0, lightIntensity*10);
        FragColor = vec4(objectColor.rgb * fade, objectColor.a);
    }})glsl";

bool running = true;
bool pause = true;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float lastX = 400.0, lastY = 300.0;
float yaw = -90;
float pitch = 0.0;
float deltaTime = 0.0;
float lastFrame = 0.0;

const double G = 6.6743e-11; // m^3 kg^-1 s^-2
const float c = 299792458.0;
float initMass = float(pow(10, 22));
float sizeRatio = 30000.0f;

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource);
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
//void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
glm::vec3 sphericalToCartesian(float r, float theta, float phi);
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount);


class Object {
public:
    GLuint VAO, VBO;
    glm::vec3 position = glm::vec3(400, 300, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    size_t vertexCount;
    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    bool Initalizing = false;
    bool Launched = false;
    bool target = false;

    float mass;
    float density;  // kg / m^3  HYDROGEN
    float radius;

    glm::vec3 LastPos = position;
    bool glow;

    Object(glm::vec3 initPosition, glm::vec3 initVelocity, float mass, float density = 3344, glm::vec4 color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), bool Glow = false) {
        this->position = initPosition;
        this->velocity = initVelocity;
        this->mass = mass;
        this->density = density;
        this->radius = pow(((3 * this->mass / this->density) / (4 * 3.14159265359)), (1.0f / 3.0f)) / sizeRatio;
        this->color = color;
        this->glow = Glow;


        // generate vertices (centered at origin)
        std::vector<float> vertices = Draw();
        vertexCount = vertices.size();

        CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount);
    }

    std::vector<float> Draw() {
        std::vector<float> vertices;
        int stacks = 10;
        int sectors = 10;

        // generate circumference points using integer steps
        for (float i = 0.0f; i <= stacks; ++i) {
            float theta1 = (i / stacks) * glm::pi<float>();
            float theta2 = (i + 1) / stacks * glm::pi<float>();
            for (float j = 0.0f; j < sectors; ++j) {
                float phi1 = j / sectors * 2 * glm::pi<float>();
                float phi2 = (j + 1) / sectors * 2 * glm::pi<float>();
                glm::vec3 v1 = sphericalToCartesian(this->radius, theta1, phi1);
                glm::vec3 v2 = sphericalToCartesian(this->radius, theta1, phi2);
                glm::vec3 v3 = sphericalToCartesian(this->radius, theta2, phi1);
                glm::vec3 v4 = sphericalToCartesian(this->radius, theta2, phi2);

                // Triangle 1: v1-v2-v3
                vertices.insert(vertices.end(), { v1.x, v1.y, v1.z }); //      /|
                vertices.insert(vertices.end(), { v2.x, v2.y, v2.z }); //     / |
                vertices.insert(vertices.end(), { v3.x, v3.y, v3.z }); //    /__|

                // Triangle 2: v2-v4-v3
                vertices.insert(vertices.end(), { v2.x, v2.y, v2.z });
                vertices.insert(vertices.end(), { v4.x, v4.y, v4.z });
                vertices.insert(vertices.end(), { v3.x, v3.y, v3.z });
            }
        }
        return vertices;
    }

    void UpdatePos() {
        this->position[0] += this->velocity[0] / 94;
        this->position[1] += this->velocity[1] / 94;
        this->position[2] += this->velocity[2] / 94;
        this->radius = pow(((3 * this->mass / this->density) / (4 * 3.14159265359)), (1.0f / 3.0f)) / sizeRatio;
    }
    void UpdateVertices() {
        // generate new vertices with current radius
        std::vector<float> vertices = Draw();

        // update VBO with new vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    }
    glm::vec3 GetPos() const {
        return this->position;
    }
    void accelerate(float x, float y, float z) {
        this->velocity[0] += x / 96;
        this->velocity[1] += y / 96;
        this->velocity[2] += z / 96;
    }
    float CheckCollision(const Object& other) {
        float dx = other.position[0] - this->position[0];
        float dy = other.position[1] - this->position[1];
        float dz = other.position[2] - this->position[2];
        float distance = std::pow(dx * dx + dy * dy + dz * dz, (1.0f / 2.0f));
        if (other.radius + this->radius > distance) {
            return -0.2f;
        }
        return 1.0f;
    }
};
std::vector<Object> objs = {};

std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object>& objs);
std::vector<float> UpdateGridVertices(std::vector<float> vertices, const std::vector<Object>& objs);

GLuint gridVAO, gridVBO;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW, panic" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Universe Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return -1;
    }

#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    glUseProgram(shaderProgram);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int screen_width = mode->width;
    int screen_height = mode->height;


    const int base_width = 1920;
    const int base_height = 1080;

    float dpi_scale = (float)screen_width / base_width;

    io.FontGlobalScale = dpi_scale;

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 750000.0f);
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    cameraPos = glm::vec3(0.0f, 1000.0f, 5000.0f);


    objs = {
       Object(glm::vec3(5844, 0, 0), glm::vec3(0, 0, 228), 7.34767309*pow(10, 22), 3344),
       //Object(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 1.989 * pow(10, 30), 5515, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f)),
       //Object(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 5.97219*pow(10, 24), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)),
       //Object(glm::vec3(-5000, 650, -350), glm::vec3(0, 0, 1500), 5.97219 * pow(10, 22), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)),
       //Object(glm::vec3(5000, 650, -350), glm::vec3(0, 0, -1500), 5.97219 * pow(10, 22), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)),
       Object(glm::vec3(0, 0, -350), glm::vec3(0, 0, 0), 1.989 * pow(10, 25), 5515, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), true),

    };
    std::vector<float> gridVertices = CreateGridVertices(20000.0f, 25, objs);
    CreateVBOVAO(gridVAO, gridVBO, gridVertices.data(), gridVertices.size());

    while (!glfwWindowShouldClose(window) && running == true) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGuiIO& io = ImGui::GetIO();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Simulation Controls");

        ImGui::Checkbox("Pause Simulation", &pause);
        ImGui::SliderFloat3("Camera Position", glm::value_ptr(cameraPos), -10000.0f, 10000.0f);

        if (ImGui::Button("Spawn Object")) {
            objs.emplace_back(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 1e22);
            objs.back().Initalizing = true;
        }

        ImGui::Text("Objects");
        ImGui::Separator();

        for (size_t i = 0; i < objs.size(); ++i) {
            ImGui::PushID(i); // Ensure unique IDs for ImGui widgets

            std::string objLabel = "Object " + std::to_string(i);
            if (ImGui::CollapsingHeader(objLabel.c_str())) {

                // Position Editor
                ImGui::Text("Position");
                ImGui::DragFloat3("##Position", glm::value_ptr(objs[i].position), 10.0f, -10000.0f, 10000.0f);

                // Velocity Editor
                ImGui::Text("Velocity");
                ImGui::DragFloat3("##Velocity", glm::value_ptr(objs[i].velocity), 1.0f, -1000.0f, 1000.0f);

                // Mass Editor
                ImGui::Text("Mass");
                ImGui::DragFloat("##Mass", &objs[i].mass, 1e20, 1e10, 1e30, "%.3e");

                // Density Editor
                ImGui::Text("Density");
                ImGui::DragFloat("##Density", &objs[i].density, 10.0f, 100.0f, 10000.0f);

                // Color Editor
                ImGui::Text("Color");
                ImGui::ColorEdit4("##Color", glm::value_ptr(objs[i].color));

                // Glow Toggle
                ImGui::Checkbox("Glow", &objs[i].glow);

                // Delete Object Button
                if (ImGui::Button("Delete Object")) {
                    objs.erase(objs.begin() + i);
                    ImGui::PopID();
                    break; // Prevent iterating over modified vector
                }
            }

            ImGui::PopID();
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSetKeyCallback(window, keyCallback);
        //glfwSetMouseButtonCallback(window, mouseButtonCallback);
        UpdateCam(shaderProgram, cameraPos);
        if (!objs.empty() && objs.back().Initalizing) {
            /*if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                // increase mass by 1% per second
                objs.back().mass *= 1.0 + 1.0 * deltaTime;

                // update radius based on new mass
                objs.back().radius = pow(
                    (3 * objs.back().mass / objs.back().density) /
                    (4 * 3.14159265359f),
                    1.0f / 3.0f
                ) / sizeRatio;

                // update vertex data
                objs.back().UpdateVertices();
            }*/
        }

        // Draw the grid
        glUseProgram(shaderProgram);
        glUniform4f(objectColorLoc, 1.0f, 1.0f, 1.0f, 0.25f);
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1);
        glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0);
        gridVertices = UpdateGridVertices(gridVertices, objs);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);
        DrawGrid(shaderProgram, gridVAO, gridVertices.size());

        // Draw the triangles / sphere
        for (auto& obj : objs) {
            glUniform4f(objectColorLoc, obj.color.r, obj.color.g, obj.color.b, obj.color.a);

            for (auto& obj2 : objs) {
                if (&obj2 != &obj && !obj.Initalizing && !obj2.Initalizing) {
                    float dx = obj2.GetPos()[0] - obj.GetPos()[0];
                    float dy = obj2.GetPos()[1] - obj.GetPos()[1];
                    float dz = obj2.GetPos()[2] - obj.GetPos()[2];
                    float distance = sqrt(dx * dx + dy * dy + dz * dz);

                    if (distance > 0) {
                        std::vector<float> direction = { dx / distance, dy / distance, dz / distance };
                        distance *= 1000;
                        double Gforce = (G * obj.mass * obj2.mass) / (distance * distance);


                        float acc1 = Gforce / obj.mass;
                        std::vector<float> acc = { direction[0] * acc1, direction[1] * acc1, direction[2] * acc1 };
                        if (!pause) {
                            obj.accelerate(acc[0], acc[1], acc[2]);
                        }

                        //collision
                        obj.velocity *= obj.CheckCollision(obj2);
                        std::cout << "radius: " << obj.radius << std::endl;
                    }
                }
            }
            if (obj.Initalizing) {
                obj.radius = pow(((3 * obj.mass / obj.density) / (4 * 3.14159265359)), (1.0f / 3.0f)) / 1000000;
                obj.UpdateVertices();
            }

            //update positions
            if (!pause) {
                obj.UpdatePos();
            }

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position); // apply position
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 0);
            if (obj.glow) {
                glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 1);
            }
            else {
                glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0);
            }

            glBindVertexArray(obj.VAO);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount / 3);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto& obj : objs) {
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
    }

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);

    glDeleteProgram(shaderProgram);
    glfwTerminate();

    glfwTerminate();
    return 0;
}

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos) {
    glUseProgram(shaderProgram);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float cameraSpeed = 10000.0f * deltaTime;
    bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
    Object& lastObj = objs[objs.size() - 1];

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        static bool paused = true;
        pause = !pause;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        objs.push_back(Object(glm::vec3(5844, 0, 0), glm::vec3(0, 0, 228), 7.34767309 * pow(10, 22), 3344));
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        static bool locked = true;
        locked = !locked;
        glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }


    // init arrows pos up down left right
    if (!objs.empty() && objs[objs.size() - 1].Initalizing) {
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            if (!shiftPressed) {
                objs[objs.size() - 1].position[1] += objs[objs.size() - 1].radius * 0.2;
            }
        };
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            if (!shiftPressed) {
                objs[objs.size() - 1].position[1] -= objs[objs.size() - 1].radius * 0.2;
            }
        }
        if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size() - 1].position[0] += objs[objs.size() - 1].radius * 0.2;
        };
        if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size() - 1].position[0] -= objs[objs.size() - 1].radius * 0.2;
        };
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size() - 1].position[2] += objs[objs.size() - 1].radius * 0.2;
        };

        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size() - 1].position[2] -= objs[objs.size() - 1].radius * 0.2;
        }
    };

};
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

/*void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            objs.emplace_back(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, 0.0f), initMass);
            objs[objs.size() - 1].Initalizing = true;
        };
        if (action == GLFW_RELEASE) {
            objs[objs.size() - 1].Initalizing = false;
            objs[objs.size() - 1].Launched = true;
        };
    };
    if (!objs.empty() && button == GLFW_MOUSE_BUTTON_RIGHT && objs[objs.size() - 1].Initalizing) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            objs[objs.size() - 1].mass *= 1.2;
        }
        std::cout << "MASS: " << objs[objs.size() - 1].mass << std::endl;
    }
};*/

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float cameraSpeed = 250000.0f * deltaTime;
    if (yoffset > 0) {
        cameraPos += cameraSpeed * cameraFront;
    }
    else if (yoffset < 0) {
        cameraPos -= cameraSpeed * cameraFront;
    }
}

glm::vec3 sphericalToCartesian(float r, float theta, float phi) {
    float x = r * sin(theta) * cos(phi);
    float y = r * cos(theta);
    float z = r * sin(theta) * sin(phi);
    return glm::vec3(x, y, z);
};

void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount) {
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for the grid
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gridVAO);
    glPointSize(5.0f);
    glDrawArrays(GL_LINES, 0, vertexCount / 3);
    glBindVertexArray(0);
}
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object>& objs) {
    std::vector<float> vertices;
    float step = size / divisions;
    float halfSize = size / 2.0f;

    // x axis
    for (int yStep = 3; yStep <= 3; ++yStep) {
        float y = -halfSize * 0.3f + yStep * step;
        for (int zStep = 0; zStep <= divisions; ++zStep) {
            float z = -halfSize + zStep * step;
            for (int xStep = 0; xStep < divisions; ++xStep) {
                float xStart = -halfSize + xStep * step;
                float xEnd = xStart + step;
                vertices.push_back(xStart); vertices.push_back(y); vertices.push_back(z);
                vertices.push_back(xEnd);   vertices.push_back(y); vertices.push_back(z);
            }
        }
    }
    // zaxis
    for (int xStep = 0; xStep <= divisions; ++xStep) {
        float x = -halfSize + xStep * step;
        for (int yStep = 3; yStep <= 3; ++yStep) {
            float y = -halfSize * 0.3f + yStep * step;
            for (int zStep = 0; zStep < divisions; ++zStep) {
                float zStart = -halfSize + zStep * step;
                float zEnd = zStart + step;
                vertices.push_back(x); vertices.push_back(y); vertices.push_back(zStart);
                vertices.push_back(x); vertices.push_back(y); vertices.push_back(zEnd);
            }
        }
    }



    return vertices;
}
std::vector<float> UpdateGridVertices(std::vector<float> vertices, const std::vector<Object>& objs) {

    // centre of mass calc
    float totalMass = 0.0f;
    float comY = 0.0f;
    for (const auto& obj : objs) {
        if (obj.Initalizing) continue;
        comY += obj.mass * obj.position.y;
        totalMass += obj.mass;
    }
    if (totalMass > 0) comY /= totalMass;

    float originalMaxY = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < vertices.size(); i += 3) {
        originalMaxY = std::max(originalMaxY, vertices[i + 1]);
    }

    float verticalShift = comY - originalMaxY;
    std::cout << "vertical shift: " << verticalShift << " |         comY: " << comY << "|            originalmaxy: " << originalMaxY << std::endl;


    for (int i = 0; i < vertices.size(); i += 3) {

        // mass bending space
        glm::vec3 vertexPos(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec3 totalDisplacement(0.0f);
        for (const auto& obj : objs) {
            //f (obj.Initalizing) continue;

            glm::vec3 toObject = obj.GetPos() - vertexPos;
            float distance = glm::length(toObject);
            float distance_m = distance * 1000.0f;
            float rs = (2 * G * obj.mass) / (c * c);

            float dz = 2 * sqrt(rs * (distance_m - rs));
            totalDisplacement.y += dz * 2.0f;
        }
        vertices[i + 1] = totalDisplacement.y + -abs(verticalShift);
    }

    return vertices;
}
