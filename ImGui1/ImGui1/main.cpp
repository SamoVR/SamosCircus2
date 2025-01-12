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

#include <implot.h>
#include <implot_internal.h>

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
#include <random>

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

ImVec4 background_color = ImVec4(0.45f, 0.55f, 0.6f, 1.0f);
bool show_histogram_window = false; // Control visibility of histogram editor

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ImGui Functions

/*void DrawHistogramsWithLine(
    const std::vector<std::vector<float>>& histograms,
    int array_count,  // Number of arrays to visualize
    float min_value,
    float max_value,
    ImVec2 size
) {
    size.x = std::max(size.x, 1.0f);
    size.y = std::max(size.y, 1.0f);

    // Combine all histograms into one for visualization
    std::vector<float> combined_histogram(array_count, 0.0f);

    // Add the data from each array
    for (const auto& array : histograms) {
        if (array.size() != array_count) {
            std::cerr << "Error: Array size mismatch!" << std::endl;
            continue;
        }

        for (int i = 0; i < array_count; ++i) {
            combined_histogram[i] += array[i]; // Add values to the histogram
            
        }
    }

    // Draw combined histogram bars
    ImGui::PlotHistogram(
        "Combined Histogram",
        combined_histogram.data(),
        array_count,
        0,
        nullptr,
        min_value,
        max_value,
        size
    );

    // Visualize additional lines over histogram bars (data lines)
    std::vector<float> data_lines(array_count);
    for (int i = 0; i < array_count; ++i) {
        data_lines[i] = combined_histogram[i]; // Get the histogram value
    }
    ImGui::PlotLines(
        "Data Lines",
        data_lines.data(),
        array_count,
        0,
        nullptr,
        min_value,
        max_value,
        size
    );
}*/

// Histogram - ImPlot version
void DrawHistogramsWithLine(
    const std::vector<std::vector<float>>& histograms,
    int array_count,  // Number of bins in the histogram
    float min_value,  // Minimum x-axis value
    float max_value,  // Maximum x-axis value
    ImVec2 size       // Size of the plot
) {
    size.x = std::max(size.x, 1.0f);
    size.y = std::max(size.y, 1.0f);

    // Combine all histograms into one for visualization
    std::vector<float> combined_histogram(array_count, 0.0f);

    // Accumulate data from all histograms
    for (const auto& array : histograms) {
        if (array.size() != array_count) {
            std::cerr << "Error: Array size mismatch!" << std::endl;
            continue;
        }

        for (int i = 0; i < array_count; ++i) {
            combined_histogram[i] += array[i];
        }
    }

    // Calculate the y-axis maximum for proper scaling
    float y_max = *std::max_element(combined_histogram.begin(), combined_histogram.end());

    // Begin ImPlot context
    if (ImPlot::BeginPlot("Histogram Plot", size)) {
        // Set up axes and their limits
        ImPlot::SetupAxes("Bins", "Frequency", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, min_value, max_value, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0f, y_max + 1.0f, ImPlotCond_Always);

        // Calculate bar width and offset
        float bar_width = (max_value - min_value) / array_count;
        float bar_offset = bar_width / 2.0f;

        // Plot combined histogram as bars
        ImPlot::PlotBars("Histogram", combined_histogram.data(), array_count, bar_width, bar_offset);

        // Overlay line graph based on histogram data
        ImPlot::PlotLine("Data Line", combined_histogram.data(), array_count);

        // End ImPlot context
        ImPlot::EndPlot();
    }
}

float GetRandomFloat(float min_val = 0.0f, float max_val = 1.0f) {
    // Create a random number generator
    std::random_device rd;  // Seed
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<float> dist(min_val, max_val); // Range [min_val, max_val]

    // Return a random float
    return dist(gen);
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

    GLFWwindow* window = glfwCreateWindow(1200, 800, "ImGui Tests", NULL, NULL);
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

    ImFont* headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 20.0f);
    ImFont* heading2Font = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 16.0f);

    if (headingFont == nullptr)
    {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Prepare histogram data
    int array_count = 8;  // Default number of histogram bins
    std::vector<std::vector<float>> histograms(1, std::vector<float>(array_count, 0.0f));
    bool show_histogram_window = false;

    /*for (auto& val : histograms[0]) {
        val = GetRandomFloat(0.0f, 1.0f); // Assign random value
    }*/

    // Initialize ImPlot context at the start of your program
    ImPlot::CreateContext();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Set OpenGL background color based on user selection
        glClearColor(background_color.x, background_color.y, background_color.z, background_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main Dashboard Window
        ImGui::Begin("Dashboard");

        // Math Section
        if (ImGui::CollapsingHeader("Math")) {

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Histogram");
            ImGui::PopFont();

            ImGui::Separator();

            // Slider for array count (dynamic bin resizing)
            static int new_array_count = array_count;
            ImGui::SliderInt("Array Count", &new_array_count, 1, 32);

            if (new_array_count != array_count) {
                array_count = new_array_count;
                histograms.resize(1);
                histograms[0].resize(array_count, 0.0f);
            }

            if (ImGui::Button("Edit Histogram Values")) {
                show_histogram_window = !show_histogram_window;  // Toggle window visibility
            }

            ImGui::Separator();

            ImGui::PushFont(heading2Font);
            ImGui::Text("Quick Actions");
            ImGui::PopFont();

            if (ImGui::Button("Randomize Histogram Values")) {
                for (auto& val : histograms[0]) {
                    val = GetRandomFloat(0.0f, 1.0f); // Assign random value
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Set All Histogram Values to 0")) {
                for (auto& val : histograms[0]) {
                    val = 0.0f;
                }
            }

            ImGui::Separator();

            // Visualize histograms with ImPlot
            DrawHistogramsWithLine(histograms, array_count, 0.0f, static_cast<float>(array_count), ImVec2(600, 200));

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);



        }

        // Background Controls Section
        if (ImGui::CollapsingHeader("Background Controls")) {

            ImGui::Separator();

            ImGui::ColorEdit3("Background Color", (float*)&background_color);

            ImGui::Separator();

        }

        ImGui::End();

        // Histogram Editor Window
        if (show_histogram_window) {
            ImGui::Begin("Histogram Values", &show_histogram_window);

            for (int j = 0; j < array_count; ++j) {
                ImGui::SliderFloat(("Value " + std::to_string(j)).c_str(),
                    &histograms[0][j], 0.0f, 1.0f);
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}
