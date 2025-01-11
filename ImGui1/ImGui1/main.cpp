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
#include <vector>

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

// Global Variables

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ImGui Functions

void DrawHistogramsWithLine(
    const std::vector<std::vector<float>>& histograms,
    const char* labels[],
    int group_count,
    float min_value,
    float max_value,
    ImVec2 size)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = (size.x == 0) ? ImVec2(ImGui::GetContentRegionAvail().x, size.y) : size;

    float group_width = canvas_size.x / group_count;
    int bar_count = histograms[0].size();
    float bar_width = group_width / histograms.size();

    // Loop through groups
    for (int g = 0; g < group_count; ++g) {
        // Draw histograms for each group
        for (int i = 0; i < histograms.size(); ++i) {
            float value = histograms[i][g];
            float bar_height = ((value - min_value) / (max_value - min_value)) * canvas_size.y;

            ImVec2 bar_min = ImVec2(pos.x + g * group_width + i * bar_width, pos.y + canvas_size.y - bar_height);
            ImVec2 bar_max = ImVec2(bar_min.x + bar_width - 1, pos.y + canvas_size.y);

            draw_list->AddRectFilled(bar_min, bar_max, IM_COL32(100 + i * 10, 150 + i * 10, 250, 255));
        }
    }

    // Draw labels below each group
    for (int g = 0; g < group_count; ++g) {
        ImVec2 label_pos = ImVec2(pos.x + g * group_width + group_width * 0.5f, pos.y + canvas_size.y + 4);
        draw_list->AddText(label_pos, IM_COL32(255, 255, 255, 255), labels[g]);
    }

    // Overlay line graph
    for (int g = 1; g < group_count; ++g) {
        for (int i = 0; i < histograms.size(); ++i) {
            float y1 = pos.y + canvas_size.y - ((histograms[i][g - 1] - min_value) / (max_value - min_value)) * canvas_size.y;
            float y2 = pos.y + canvas_size.y - ((histograms[i][g] - min_value) / (max_value - min_value)) * canvas_size.y;

            float x1 = pos.x + (g - 1) * group_width + group_width * 0.5f;
            float x2 = pos.x + g * group_width + group_width * 0.5f;

            draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 0, 0, 255), 2.0f);
        }
    }

    // Draw border
    draw_list->AddRect(pos, ImVec2(pos.x + canvas_size.x, pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));
    ImGui::Dummy(ImVec2(canvas_size.x, canvas_size.y + 20)); // Reserve space
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui Tests", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    io.Fonts->AddFontDefault(); //ProggyClean

    ImFont* headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (headingFont == nullptr)
    {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Prepare histogram data
    const int group_count = 16; // Number of groups
    const int array_count = 3;  // Number of arrays
    std::vector<std::vector<float>> histograms(array_count, std::vector<float>(group_count, 0.0f));
    const char* labels[group_count];
    for (int i = 0; i < group_count; ++i) labels[i] = std::to_string(i).c_str();

    // Fill histogram data
    for (int i = 0; i < array_count; ++i) {
        for (int j = 0; j < group_count; ++j) {
            histograms[i][j] = sinf(j * 0.3f + i) * 0.5f + 0.5f; // Example values
        }
    }

    while (!glfwWindowShouldClose(window)) { //MAIN LOOP

        glfwPollEvents();

        // ImGui Initialize
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui Controls Window
        ImGui::Begin("Dashboard");

        if (ImGui::CollapsingHeader("Math"))
        {
            DrawHistogramsWithLine(histograms, labels, group_count, 0.0f, 1.0f, ImVec2(600, 200));

        }

        if (ImGui::CollapsingHeader("Background Controls")) 
        {
            ImGui::Separator();

            ImGui::ColorEdit3("Background Color", (float*)&clear_color);

            ImGui::Separator();

        }

        ImGui::End();

        // Clear the screen
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get framebuffer dimensions and calculate aspect ratio
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = static_cast<float>(width) / static_cast<float>(height);

        // Render ImGui UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
