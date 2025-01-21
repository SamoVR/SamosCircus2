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

// Helper functions for matrix operations
void SetToIdentity(float* matrix, int size) {
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = (i % (size + 1) == 0) ? 1.0f : 0.0f; // Diagonal elements = 1, others = 0
    }
}

float CalculateDeterminant2x2(const float* matrix) {
    return matrix[0] * matrix[3] - matrix[1] * matrix[2];
}

// Draw Matrix Input
void DrawMatrixInput(float* matrix, int size, const char* label) {
    if (size < 2 || size > 4) {
        ImGui::Text("Matrix size must be between 2x2 and 4x4");
        return;
    }

    ImGui::Text("%s (%dx%d):", label, size, size);

    ImGui::Columns(size, nullptr, false); // Create grid layout
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            ImGui::PushID(row * size + col); // Unique ID for each input box
            ImGui::InputFloat("", &matrix[row * size + col], 0.1f, 1.0f, "%.2f");
            ImGui::PopID();
            ImGui::NextColumn(); // Move to next cell
        }
    }
    ImGui::Columns(1); // Reset columns layout

    // Add quick actions
    if (ImGui::Button("Reset to Identity")) {
        SetToIdentity(matrix, size);
    }
    ImGui::SameLine();
    if (size == 2) { // Example: Calculate determinant for 2x2 matrices
        float det = CalculateDeterminant2x2(matrix);
        ImGui::Text("Determinant: %.2f", det);
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

    //Value Manipulation
    float floatValue = 0;
    int intValue = 0;

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

            ImGui::PushFont(headingFont);
            ImGui::Text("Value Manipulation");
            ImGui::PopFont();

            ImGui::Separator();

            ImGui::SliderFloat("Float Slider", &floatValue, 0, 5);
            ImGui::SliderInt("Int Slider", &intValue, 0, 5);

            ImGui::DragFloat("Drag Float", &floatValue, 0.1, 0, 5);
            ImGui::DragInt("Drag Int", &intValue, 0.1, 0, 5);

            ImGui::InputFloat("Float Input", &floatValue);
            ImGui::InputInt("Int Input", &intValue);

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Sine Wave");
            ImGui::PopFont();

            static float x_values[100], y_values[100];
            for (int i = 0; i < 100; ++i) {
                x_values[i] = i * 0.1f;
                y_values[i] = sin(x_values[i]);
            }
            if (ImPlot::BeginPlot("Sine Wave")) {
                ImPlot::PlotLine("sin(x)", x_values, y_values, 100);
                ImPlot::EndPlot();
            }

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Scatter Plot");
            ImGui::PopFont();

            static float xs[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            static float ys[10] = { 1, 4, 9, 16, 25, 36, 49, 64, 81, 100 };
            if (ImPlot::BeginPlot("Scatter Plot")) {
                ImPlot::PlotScatter("Points", xs, ys, 10);
                ImPlot::EndPlot();
            }

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Heat Map");
            ImGui::PopFont();

            static float values[5][5] = {
                {0.1f, 0.2f, 0.3f, 0.4f, 0.5f},
                {0.2f, 0.3f, 0.4f, 0.5f, 0.6f},
                {0.3f, 0.4f, 0.5f, 0.6f, 0.7f},
                {0.4f, 0.5f, 0.6f, 0.7f, 0.8f},
                {0.5f, 0.6f, 0.7f, 0.8f, 0.9f},
            };
            if (ImPlot::BeginPlot("Heatmap")) {
                ImPlot::PlotHeatmap("Heat", &values[0][0], 5, 5);
                ImPlot::EndPlot();
            }

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Pie Chart");
            ImGui::PopFont();

            float data[] = { 30.0f, 20.0f, 50.0f };
            const char* labels[] = { "A", "B", "C" };
            if (ImPlot::BeginPlot("Pie Chart")) {
                ImPlot::PlotPieChart(labels, data, 3, 0.5f, 0.5f, 0.3f);
                ImPlot::EndPlot();
            }

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Matrix");
            ImGui::PopFont();

            static float matrix_3x3[9] = {
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
            };

            DrawMatrixInput(matrix_3x3, 3, "3x3 Matrix");

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);

            ImGui::PushFont(headingFont);
            ImGui::Text("Polynomial Solver");
            ImGui::PopFont();

            // Polynomial solver example: ax^2 + bx + c
            static float coefficients[3] = { 1.0f, -3.0f, 2.0f }; // Coefficients a, b, c
            static float x_value = 1.0f; // Value for evaluation

            ImGui::InputFloat3("Coefficients (a, b, c)", coefficients);
            ImGui::SliderFloat("Evaluate at x", &x_value, -10.0f, 10.0f);

            float result = coefficients[0] * x_value * x_value + coefficients[1] * x_value + coefficients[2];
            ImGui::Text("f(x) = %.2f", result);


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
