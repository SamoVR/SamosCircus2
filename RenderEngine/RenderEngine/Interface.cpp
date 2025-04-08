#include "Interface.h"

Interface::Interface(GLFWwindow* window, Scene& scene) : window(window), scene(scene) {
    // Enable OpenGL blend mode for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load font
    io.FontGlobalScale = 1.0f;
    io.Fonts->AddFontDefault();
    this->headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (headingFont == nullptr)
    {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();
}

Interface::~Interface() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Interface::update() {
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create the UI
    createUI();

    // End the ImGui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::createUI() {
    // Create a window to display the UI
    ImGui::Begin("Scene Object Controls");

    displayObjectList();  // Display all objects in the scene

    ImGui::End();
}

void Interface::displayObjectList() {
    // Display the list of objects in the scene
    ImGui::Text("Objects in Scene:");

    for (size_t i = 0; i < scene.getObjects().size(); ++i) {
        Object& object = scene.getObjects()[i];

        // Create a collapsing header for each object
        if (ImGui::CollapsingHeader(("Object " + std::to_string(i)).c_str())) {
            // Display object properties inside the header's collapsed section
            displayObjectProperties(object);
        }
    }
}

void Interface::displayObjectProperties(Object& object) {
    // Display the properties of the selected object

    ImGui::Text("Selected Object:");

    // Display position with sliders
    ImGui::Text("Position:");
    if (ImGui::SliderFloat3("Position", glm::value_ptr(object.position), -10.0f, 10.0f)) {
        object.setPosition(object.position);  // Apply changes to position
    }

    // Display rotation with sliders
    ImGui::Text("Rotation:");
    if (ImGui::SliderFloat3("Rotation", glm::value_ptr(object.rotation), -180.0f, 180.0f)) {
        object.setRotation(object.rotation);  // Apply changes to rotation
    }

    // Display scale with sliders
    ImGui::Text("Scale:");
    if (ImGui::SliderFloat3("Scale", glm::value_ptr(object.scale), 0.1f, 5.0f)) {
        object.setScale(object.scale);  // Apply changes to scale
    }
}
