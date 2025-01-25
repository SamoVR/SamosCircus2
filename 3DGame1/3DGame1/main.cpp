#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

// Vertex Shader source
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;  // Pass the color to the fragment shader
}
)";

// Fragment Shader source
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0); // Use the interpolated color
}
)";

// Room vertices (simple cube room)
float roomVertices[] = {
    // Floor (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,  // Grey
     5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,
     5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,
    -5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,
     5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,
    -5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,

    // Back Wall (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,  // Blue
     5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,
    -5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,
    -5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,

    // Front Wall (2 triangles)
    -5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,  // Red
     5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,
     5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,
    -5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,
     5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,

    // Left Wall (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.0f, 1.0f, 0.0f,  // Green
    -5.0f, -1.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f, -1.0f, -5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f, -5.0f, 0.0f, 1.0f, 0.0f,

    // Right Wall (2 triangles)
    10.0f, -1.0f, -5.0f, 1.0f, 0.0f, 1.0f,  // Pink
    10.0f, -1.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f, -1.0f, -5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f, -5.0f, 1.0f, 0.0f, 1.0f,

};

const int width = 800, height = 600;

glm::vec3 cameraPos(0.0f, 2.0f, 0.0f);   // Initial camera position
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f); // Direction camera is looking
glm::vec3 characterFront(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);     // Up direction

float yaw = -90.0f;  // Horizontal rotation
float pitch = 0.0f;  // Vertical rotation
float fov = 60.0f;   // Field of view
float sensitivity = 0.1f;  // Mouse sensitivity
bool firstMouse = true;
float lastX = width / 2.0f, lastY = height / 2.0f;

// Character position
glm::vec3 characterPos(0.0f, 0.0f, 0.0f);
float characterSpeed = 10.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch = glm::clamp(pitch + yoffset, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    glm::vec3 front2;
    front2.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front2.y = 0.0f;
    front2.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    characterFront = glm::normalize(front2);
}

// Initialize the GLFW window
GLFWwindow* initWindow(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Disable cursor for FPS control
    return window;
}

// Compile shader and create program
GLuint compileShader(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Process input for movement
void processInput(GLFWwindow* window, float deltaTime) {
    float velocity = characterSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += characterFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= characterFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    if (glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

}


// Main function
int main() {
    GLFWwindow* window = initWindow(width, height, "3DGame1");
    if (!window) return -1;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint shaderProgram = compileShader(vertexShaderSource, fragmentShaderSource);

    // Setup room VAO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window, deltaTime);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glClearColor(0.2, 0.2, 0.3,0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render room
        glUseProgram(shaderProgram);

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / height, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 30); // 6 triangles, 18 vertices



        // Render ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Controls");
        
        ImGui::SliderFloat("Character Speed", &characterSpeed,1.0f,20.0f);
        ImGui::SliderFloat("FOV", &fov, 1.0f, 90.0f);

        ImGui::End();

        ImGui::Begin("Debug Info");

        ImGui::Text("Camera");
        ImGui::Separator();
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
        ImGui::Text("Camera Front: (%.2f, %.2f, %.2f)", cameraFront.x, cameraFront.y, cameraFront.z);

        ImGui::Text("Yaw: %f",yaw);
        ImGui::Text("Pitch: %f", pitch);
        ImGui::Text("FOV: %f", fov);
        ImGui::Text("Sensitivity: %f", sensitivity);
        ImGui::Text("firstMouse: %f", firstMouse);
        ImGui::Text("lastX: %f | lastY: %f", lastX,lastY);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
