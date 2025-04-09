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
    ImGui::Begin("Scene Object Controls");

    // Add Object Button
    if (ImGui::Button("Add Object")) {
        scene.addObject(new Object("PlaceHolderObject", createCubeVertices(), nullptr));  // Add a new default object to the scene
    }

    ImGui::Separator();

    displayObjectList();  // Display all objects in the scene

    ImGui::End();
}

void Interface::displayObjectList() {
    ImGui::Text("Objects in Scene:");

    // Track index to remove after the loop
    int indexToRemove = -1;

    for (size_t i = 0; i < scene.getObjects().size(); ++i) {
        Object* object = scene.getObjects()[i];
        std::string headerLabel = "Object " + std::to_string(i);

        if (ImGui::CollapsingHeader(headerLabel.c_str())) {
            displayObjectProperties(object, static_cast<int>(i));  // <-- Pass index
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Button("Remove Object")) {
                indexToRemove = static_cast<int>(i);
            }
            ImGui::PopID();
        }
    }


    // Remove object *after* the loop to avoid iterator invalidation
    if (indexToRemove >= 0) {
        scene.removeObject(static_cast<size_t>(indexToRemove));
    }
}

void Interface::displayObjectProperties(Object* object, int index) {
    ImGui::PushID(index);  // Ensure uniqueness of ImGui widgets

    ImGui::InputFloat3("Position", glm::value_ptr(object->position));
    
    ImGui::SliderFloat3("Rotation", glm::value_ptr(object->rotation), -180.0f, 180.0f);

    ImGui::SliderFloat3("Scale", glm::value_ptr(object->scale), 0.1f, 5.0f);

    ImGui::PopID();
}

