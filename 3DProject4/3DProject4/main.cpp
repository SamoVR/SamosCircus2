// Samo_VR
// Rotating cube with expanded 3D space control
//TODO: make file "textures" be included in output app

//GLFW
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

//GLM
#define GLM_ENABLE_EXPERIMENTAL

//STB
#define STB_IMAGE_IMPLEMENTATION

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <stb_image.h>

#include <string>
#include <iostream>

// Shaders

const char* vertexShaderSource = R"(
    #version 330 core

    layout(location = 0) in vec3 aPos;         // Vertex position
    layout(location = 1) in vec3 aColor;       // Vertex color
    layout(location = 2) in float isFloor;     // isFloor flag
    layout(location = 3) in vec2 aTexCoord;    // Texture coordinates

    out vec3 vertexColor;  // Pass vertex color to the fragment shader
    out vec2 TexCoord;
    out float isFloorFlag;

    uniform mat4 mvp;

    void main()
    {
        gl_Position = mvp * vec4(aPos, 1.0);
        vertexColor = aColor;  // Pass the color to the fragment shader
        TexCoord = aTexCoord;
        isFloorFlag = isFloor;
    }
)";


const char* fragmentShaderSource = R"(
    #version 330 core

    in vec3 vertexColor;  // Interpolated vertex color
    in vec2 TexCoord;
    in float isFloorFlag;

    out vec4 FragColor;

    uniform sampler2D floorTexture;

    void main()
    {
        vec4 texColor = texture(floorTexture, TexCoord);

        // Use a conditional check to blend colors only for non-floor surfaces
        if (isFloorFlag < 0.5) {
            FragColor = vec4(vertexColor, 1.0);  // Combine vertex color for non-floor surfaces
        } else {
            FragColor = texColor;  // Use only the texture color for the floor
        }
    }
)";

// Global variables

// Camera parameters
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);  // Camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);     // Camera front vector
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);         // Camera up vector
float cameraMovementSpeed = 0.05f;
bool cameraMovementEnabled = false;

// Special Variables
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
glm::vec3 defaultRotationDirection = glm::vec3(0.5f, 1.0f, 0.0f);
glm::vec3 rotationDirection = defaultRotationDirection;

// Variables
//float defaultRotationSpeed = 50.0f;
float rotationSpeed = 50.0f;

float minFov = 40.0f;
float maxFov = 170.0f;
float defaultFov = 70.0f;
float fov = defaultFov; // camera fov

float rotationAngle = 0.0f;

float minDistance = 1.0f;
float maxDistance = 75.0f;
float defaultDistance = 5.0f;
float distance = defaultDistance; // camera distance

float lastX = 400.0f;
float lastY = 300.0f;
float yaw = -90.0f, pitch = 0.0f;

bool isDragging = false;

bool floorEnabled = true;

//cube manipulation
int keySetSpeed = GLFW_KEY_P;       // Default key for "Set Speed to 0"

//cam manipulation
int keyResetRotation = GLFW_KEY_E; // Default key for "Reset Rotation Directions"
int keyResetFOV = GLFW_KEY_F;      // Default key for "Reset FOV"
int keyResetDistance = GLFW_KEY_G; // Default key for "Reset Distance"
int keyResetCameraPosition = GLFW_KEY_B;

//cam movement
int keyMoveCamLeft = GLFW_KEY_A;
int keyMoveCamRight = GLFW_KEY_D;
int keyMoveCamUp = GLFW_KEY_W;
int keyMoveCamDown = GLFW_KEY_S;

//global keybind
int rebindActiveKey = -1;

// Functions

std::string ToUpper(const std::string& input) {
    std::string result = input;
    for (char& c : result) {
        c = toupper(c);  // Convert each character to uppercase
    }
    return result;
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyboard_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {

        // OTHER KEYBINDS

        if (key == keySetSpeed) {
            rotationSpeed = 0.0f;
            std::cout << "Speed reset to 0" << std::endl;
        }
        if (key == keyResetRotation) {
            rotationDirection = defaultRotationDirection;
            std::cout << "Rotation reset" << std::endl;
        }
        if (key == keyResetFOV) {
            fov = defaultFov;  // Default FOV reset
            std::cout << "FOV reset" << std::endl;
        }
        if (key == keyResetDistance) {
            distance = defaultDistance;  // Set distance back to a default value
            std::cout << "Distance reset" << std::endl;
        }
    }
}


void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Scroll up (yoffset > 0) decreases distance
    // Scroll down (yoffset < 0) increases distance
    float scrollSpeed = 2.0f;  // Adjust scroll speed as needed
    distance -= yoffset * scrollSpeed;

    if (distance < minDistance) distance = minDistance;  // Clamp to prevent too close
    if (distance > maxDistance) distance = maxDistance;  // Clamp to prevent too far

    //std::cout << "Current distance: " << distance << std::endl;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isDragging = true;

            // Get the current cursor position
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



void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;  // Y-axis is flipped

        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // Recalculate cameraFront
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}


// Main Function

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

    glfwSetKeyCallback(window, keyboard_key_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_position_callback);


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

    float floorVertices[] = {
        // Positions         // Texture Coords  // isFloor
        -10.0f, 0.0f, -10.0f,  0.0f, 0.0f, 1.0f,  // Vertex 0
         10.0f, 0.0f, -10.0f,  1.0f, 0.0f, 1.0f,  // Vertex 1
         10.0f, 0.0f,  10.0f,  1.0f, 1.0f, 1.0f,  // Vertex 2
        -10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 1.0f   // Vertex 3
    };
    unsigned int floorIndices[] = { 0, 1, 2, 2, 3, 0 };

    unsigned int floorVAO, floorVBO, floorEBO; //FLOOR
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);

    // Bind VBO and fill it with data
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    // Bind EBO and fill it with indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    // Set up Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set up Texture Coordinates attribute (location = 3)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Set up isFloor attribute
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //// VAO END

    unsigned int VBO, VAO, EBO; //CUBE
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind Vertex Array Object
    glBindVertexArray(VAO);

    // Bind and set Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind and set Element Buffer Object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Define position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Define color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //// VAO END

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

    // Texture Variables
    unsigned int floorTexture = loadTexture("assets/textures/texture_08.png");

    // Variables
    float lastFrameTime = glfwGetTime();

    io.Fonts->AddFontDefault(); //ProggyClean

    ImFont* headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (headingFont == nullptr)
    {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();

    while (!glfwWindowShouldClose(window)) { //MAIN LOOP
        float currentFrameTime = glfwGetTime();
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        // ImGui Initialize
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui Controls Window
        ImGui::Begin("Dashboard");

        if (ImGui::CollapsingHeader("Controls")) {

            ImGui::PushFont(headingFont);
            ImGui::Text("Cube Controls");
            ImGui::PopFont();

            ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.0f, 2500.0f);
            ImGui::SliderFloat3("Rotation Direction", glm::value_ptr(rotationDirection), -5.0f, 5.0f);

            ImGui::Separator();

            ImGui::PushFont(headingFont);
            ImGui::Text("Camera Controls");
            ImGui::PopFont();

            ImGui::SliderFloat("Distance", &distance, minDistance, maxDistance);
            ImGui::SliderFloat("FOV", &fov, minFov, maxFov);
            ImGui::Checkbox("Enable Free Camera", &cameraMovementEnabled);
            ImGui::SliderFloat("Camera Speed", &cameraMovementSpeed, 0.001f, 1.0f);

            ImGui::Separator();

            ImGui::PushFont(headingFont);
            ImGui::Text("Background Controls");
            ImGui::PopFont();

            ImGui::ColorEdit3("Clear Color", (float*)&clear_color);
            ImGui::Checkbox("Enable Floor", &floorEnabled);

            ImGui::Separator();

            ImGui::PushFont(headingFont);
            ImGui::Text("Cube's Face Colors");
            ImGui::PopFont();

            ImGui::ColorEdit3("Face 1 Color", (float*)&color1);
            ImGui::ColorEdit3("Face 2 Color", (float*)&color2);
            ImGui::ColorEdit3("Face 3 Color", (float*)&color3);
            ImGui::ColorEdit3("Face 4 Color", (float*)&color4);
            ImGui::ColorEdit3("Face 5 Color", (float*)&color5);
            ImGui::ColorEdit3("Face 6 Color", (float*)&color6);

        }

        if (ImGui::CollapsingHeader("Keybinds")) {

            // Function to create a rebindable keybind button
            auto renderRebindButton = [&](const char* label, int& key, int keyId) {
                ImGui::Text(label);
                ImGui::SameLine();

                // Get the key name and convert to uppercase
                const char* keyName = glfwGetKeyName(key, 0);
                std::string buttonLabel = (rebindActiveKey == keyId)
                    ? "Press a key..."
                    : ToUpper(keyName).c_str();

                // Show the button and handle rebinding
                if (ImGui::Button(buttonLabel.c_str())) {
                    rebindActiveKey = keyId; // Set this key for rebinding
                }

                // Handle rebinding when this button is active
                if (rebindActiveKey == keyId) {
                    // Iterate through possible keys and detect key press
                    for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i) {
                        if (glfwGetKey(window, i) == GLFW_PRESS) {
                            key = i;             // Assign the new key
                            rebindActiveKey = -1; // Exit rebinding mode
                        }
                    }
                }
                };


            // Render the keybind buttons
            ImGui::PushFont(headingFont);
            ImGui::Text("Cube Manipulation Keybinds");
            ImGui::PopFont();

            ImGui::Separator();

            renderRebindButton("Set Speed to 0:", keySetSpeed, 1);


            ImGui::PushFont(headingFont);
            ImGui::Text("Camera Manipulation Keybinds");
            ImGui::PopFont();

            ImGui::Separator();

            renderRebindButton("Reset Rotation Directions:", keyResetRotation, 2);
            renderRebindButton("Reset FOV:", keyResetFOV, 3);
            renderRebindButton("Reset Distance:", keyResetDistance, 4);
            //renderRebindButton("Reset Position:", keyResetCameraPosition, 5);

            ImGui::PushFont(headingFont);
            ImGui::Text("Camera Movement Keybinds");
            ImGui::PopFont();
            
            ImGui::Separator();

            renderRebindButton("Camera Move Left:", keyMoveCamLeft, 6);
            renderRebindButton("Camera Move Right:", keyMoveCamRight, 7);
            renderRebindButton("Camera Move Up:", keyMoveCamUp, 8);
            renderRebindButton("Camera Move Down:", keyMoveCamDown, 9);

        }

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

        // Update the vertex buffer if necessary
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Update buffer with new vertices
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Clear the screen
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get framebuffer dimensions and calculate aspect ratio
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = static_cast<float>(width) / static_cast<float>(height);

        // Update rotation angle
        rotationAngle += rotationSpeed * deltaTime;

        // Camera view transformation
        glm::mat4 view;

        if (cameraMovementEnabled) {
            cameraMovementSpeed * deltaTime;

            // Camera WASD movement
            if (glfwGetKey(window, keyMoveCamUp) == GLFW_PRESS)
                cameraPosition += cameraMovementSpeed * cameraFront;
            if (glfwGetKey(window, keyMoveCamDown) == GLFW_PRESS)
                cameraPosition -= cameraMovementSpeed * cameraFront;
            if (glfwGetKey(window, keyMoveCamLeft) == GLFW_PRESS)
                cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMovementSpeed;
            if (glfwGetKey(window, keyMoveCamRight) == GLFW_PRESS)
                cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMovementSpeed;

            // Construct the view matrix with camera movement
            view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        }
        else {
            float camX = distance * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            float camY = distance * sin(glm::radians(pitch));
            float camZ = distance * sin(glm::radians(yaw)) * cos(glm::radians(pitch));

            view = glm::lookAt(
                glm::vec3(camX, camY, camZ),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }

        // Projection transformation
        glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

        // Cube transformation
        glm::mat4 modelCube = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(rotationDirection));
        glm::mat4 mvpCube = projection * view * modelCube;

        // Use shader program and set the MVP matrix for the cube
        glUseProgram(shaderProgram);
        unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvpCube));

        // Bind the cube's VAO and draw it
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Transform for the floor
        glm::mat4 modelFloor = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f));
        glm::mat4 mvpFloor = projection * view * modelFloor;

        // Set the MVP matrix for the floor
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvpFloor));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "floorTexture"), 0);

        // Bind and draw the floor
        if (floorEnabled)
        {
            glBindVertexArray(floorVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        //glBindVertexArray(0);

        // Render ImGui UI
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
