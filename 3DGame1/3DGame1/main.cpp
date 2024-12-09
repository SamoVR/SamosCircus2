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
// Room vertices with color attributes (x, y, z, r, g, b)
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
};



// Character position
glm::vec3 characterPos(0.0f, 0.0f, 0.0f);
const float characterSpeed = 2.5f;

// Collision boundaries
const float boundary = 4.5f;

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
    glm::vec3 movement(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement.z -= characterSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement.z += characterSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement.x -= characterSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement.x += characterSpeed * deltaTime;

    // Update character position with boundary constraints
    characterPos += movement;
    characterPos.x = glm::clamp(characterPos.x, -boundary, boundary);
    characterPos.z = glm::clamp(characterPos.z, -boundary, boundary);
}

// Main function
int main() {
    const int width = 800, height = 600;
    GLFWwindow* window = initWindow(width, height, "Basic Game with ImGui");
    if (!window) return -1;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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


    // Projection and camera setup
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
    glm::mat4 view;
    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render room
        glUseProgram(shaderProgram);
        model = glm::translate(glm::mat4(1.0f), characterPos);
        view = glm::lookAt(glm::vec3(0.0f, 2.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18); // 6 triangles, 18 vertices



        // Render ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Info");
        ImGui::Text("Character Position: (%.2f, %.2f, %.2f)", characterPos.x, characterPos.y, characterPos.z);
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
