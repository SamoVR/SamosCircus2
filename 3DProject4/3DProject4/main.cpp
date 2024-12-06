// Samo_VR
// Rotating cube with expanded 3D space control

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

// Camera parameters
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);  // Camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);     // Camera front vector
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);         // Camera up vector

float lastX = 400.0f;
float lastY = 300.0f;
float yaw = -90.0f, pitch = 0.0f;
bool isDragging = false;



void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isDragging = true;

            // Initialize lastX and lastY to the current cursor position
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
        }
        else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
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
    }
}



int main() {
    if (!glfwInit()) return -1;

    // Decide GL+GLSL versions
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Cube with ImGui", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
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

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    ImVec4 color1 = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
    ImVec4 color2 = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green
    ImVec4 color3 = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
    ImVec4 color4 = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
    ImVec4 color5 = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);  // Magenta
    ImVec4 color6 = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);  // Cyan

    float vertices[] = {
        // Positions               // Colors (from ImVec4)
        -0.5f, -0.5f, -0.5f,  color1.x, color1.y, color1.z,
        0.5f, -0.5f, -0.5f, color1.x, color1.y, color1.z,
        0.5f, 0.5f, -0.5f, color1.x, color1.y, color1.z,
        -0.5f, 0.5f, -0.5f, color1.x, color1.y, color1.z,

        -0.5f, -0.5f, 0.5f, color2.x, color2.y, color2.z,
        0.5f, -0.5f, 0.5f, color2.x, color2.y, color2.z,
        0.5f, 0.5f, 0.5f, color2.x, color2.y, color2.z,
        -0.5f, 0.5f, 0.5f, color2.x, color2.y, color2.z,

        -0.5f, -0.5f, -0.5f, color3.x, color3.y, color3.z,
        -0.5f, -0.5f, 0.5f, color3.x, color3.y, color3.z,
        -0.5f, 0.5f, 0.5f, color3.x, color3.y, color3.z,
        -0.5f, 0.5f, -0.5f, color3.x, color3.y, color3.z,

        0.5f, -0.5f, -0.5f, color4.x, color4.y, color4.z,
        0.5f, -0.5f, 0.5f, color4.x, color4.y, color4.z,
        0.5f, 0.5f, 0.5f, color4.x, color4.y, color4.z,
        0.5f, 0.5f, -0.5f, color4.x, color4.y, color4.z,

        -0.5f, -0.5f, -0.5f, color5.x, color5.y, color5.z,
        0.5f, -0.5f, -0.5f, color5.x, color5.y, color5.z,
        0.5f, -0.5f, 0.5f, color5.x, color5.y, color5.z,
        -0.5f, -0.5f, 0.5f, color5.x, color5.y, color5.z,

        -0.5f, 0.5f, -0.5f, color6.x, color6.y, color6.z,
        0.5f, 0.5f, -0.5f, color6.x, color6.y, color6.z,
        0.5f, 0.5f, 0.5f, color6.x, color6.y, color6.z,
        -0.5f, 0.5f, 0.5f, color6.x, color6.y, color6.z
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float rotationSpeed = 50.0f;
    glm::vec3 rotationDirection = glm::vec3(0.5f, 1.0f, 0.0f);
    float distance = 45.0f; // cube distance
    float radius = 10.0f;
    float rotationAngle = 0.0f;
    float lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window
        ImGui::Begin("Controls");

        ImGui::Text("Cube Controls");
        ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0f, 2500.0f);
        ImGui::SliderFloat("Distance", &distance, 5.0f, 170.0f);
        ImGui::SliderFloat3("Rotation Direction", glm::value_ptr(rotationDirection), -5.0f, 5.0f);

        ImGui::Separator();

        ImGui::Text("Background Controls");
        ImGui::ColorEdit3("Clear Color", (float*)&clear_color);

        ImGui::Separator();

        ImGui::Text("Cube Colors");
        ImGui::ColorEdit3("Face 1 Color", (float*)&color1);
        ImGui::ColorEdit3("Face 2 Color", (float*)&color2);
        ImGui::ColorEdit3("Face 3 Color", (float*)&color3);
        ImGui::ColorEdit3("Face 4 Color", (float*)&color4);
        ImGui::ColorEdit3("Face 5 Color", (float*)&color5);
        ImGui::ColorEdit3("Face 6 Color", (float*)&color6);

        ImGui::End();

        // Front face (indices 0-3)
        vertices[3] = color1.x; vertices[4] = color1.y; vertices[5] = color1.z;
        vertices[9] = color1.x; vertices[10] = color1.y; vertices[11] = color1.z;
        vertices[15] = color1.x; vertices[16] = color1.y; vertices[17] = color1.z;
        vertices[21] = color1.x; vertices[22] = color1.y; vertices[23] = color1.z;

        // Back face (indices 4-7)
        vertices[27] = color2.x; vertices[28] = color2.y; vertices[29] = color2.z;
        vertices[33] = color2.x; vertices[34] = color2.y; vertices[35] = color2.z;
        vertices[39] = color2.x; vertices[40] = color2.y; vertices[41] = color2.z;
        vertices[45] = color2.x; vertices[46] = color2.y; vertices[47] = color2.z;

        // Left face (indices 8-11)
        vertices[51] = color3.x; vertices[52] = color3.y; vertices[53] = color3.z;
        vertices[57] = color3.x; vertices[58] = color3.y; vertices[59] = color3.z;
        vertices[63] = color3.x; vertices[64] = color3.y; vertices[65] = color3.z;
        vertices[69] = color3.x; vertices[70] = color3.y; vertices[71] = color3.z;

        // Right face (indices 12-15)
        vertices[75] = color4.x; vertices[76] = color4.y; vertices[77] = color4.z;
        vertices[81] = color4.x; vertices[82] = color4.y; vertices[83] = color4.z;
        vertices[87] = color4.x; vertices[88] = color4.y; vertices[89] = color4.z;
        vertices[93] = color4.x; vertices[94] = color4.y; vertices[95] = color4.z;

        // Bottom face (indices 16-19)
        vertices[99] = color5.x; vertices[100] = color5.y; vertices[101] = color5.z;
        vertices[105] = color5.x; vertices[106] = color5.y; vertices[107] = color5.z;
        vertices[111] = color5.x; vertices[112] = color5.y; vertices[113] = color5.z;
        vertices[117] = color5.x; vertices[118] = color5.y; vertices[119] = color5.z;

        // Top face (indices 20-23)
        vertices[123] = color6.x; vertices[124] = color6.y; vertices[125] = color6.z;
        vertices[129] = color6.x; vertices[130] = color6.y; vertices[131] = color6.z;
        vertices[135] = color6.x; vertices[136] = color6.y; vertices[137] = color6.z;
        vertices[141] = color6.x; vertices[142] = color6.y; vertices[143] = color6.z;

        // Send updated color data back to the GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Updating buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / height;

        rotationAngle += rotationSpeed * deltaTime;

        glm::mat4 view = glm::mat4(1.0f);
        float x = radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        float y = radius * sin(glm::radians(pitch));
        float z = radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Apply transformations
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(rotationDirection));

        //glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 projection = glm::perspective(glm::radians(distance), aspect, 0.1f, 100.0f);
        glm::mat4 mvp = projection * view * model;

        glUseProgram(shaderProgram);
        int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
