#include "Interface.h"

Interface::Interface(GLFWwindow* window, Scene& scene)
    : window(window), scene(scene), defaultTexture(new Texture("assets/textures/texture_08.png")) {

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // Load font
    io.FontGlobalScale = 1.0f;
    io.Fonts->AddFontDefault();
    headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (!headingFont) {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();
}

Interface::~Interface() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Interface::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // DockSpace
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);


    ImGui::Begin("MainDockspace", nullptr, windowFlags);
    ImGui::PopStyleVar(1);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; // | ImGuiDockNodeFlags_NoDockingInCentralNode

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();

    // Separate windows
    sceneControlsUI();
    objectListUI();
    settingsUI(); // Floaty

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::settingsUI() {
    static bool showSettings = true;
    if (!showSettings) return;

    ImGui::Begin("Settings", &showSettings);

    ImGui::Text("App Settings");

    ImGui::End();
}


void Interface::objectListUI() {
    ImGui::Begin("Object List");

    ImGui::PushFont(headingFont);
    ImGui::Text("Objects:");
    ImGui::PopFont();

    int indexToRemove = -1;
    for (size_t i = 0; i < scene.getObjects().size(); ++i) {
        Object* object = scene.getObjects()[i];
        std::string headerLabel = object->name;

        ImGui::PushID(static_cast<int>(i));
        if (ImGui::CollapsingHeader(headerLabel.c_str())) {
            displayObjectProperties(object, static_cast<int>(i));
            if (ImGui::Button("Remove Object")) indexToRemove = static_cast<int>(i);
        }
        ImGui::PopID();
    }

    if (indexToRemove >= 0) scene.removeObject(static_cast<size_t>(indexToRemove));

    ImGui::End();
}


void Interface::sceneControlsUI() {
    ImGui::Begin("Scene Controls");

    if (ImGui::Button("Add Object")) {
        showShapePopup = true;
        ImGui::OpenPopup("Select Object Shape");
    }

    showShapeSelectionPopup();

    ImGui::SameLine();
    if (ImGui::Button("Save Scene")) scene.saveToFile("scene.json");
    ImGui::SameLine();
    if (ImGui::Button("Load Scene")) scene.loadFromFile("scene.json");

    ImGui::End();
}

void Interface::showShapeSelectionPopup() {
    // Open the popup when necessary
    if (showShapePopup) { // Make sure this flag is set when you want to show the popup
        if (ImGui::BeginPopupModal("Select Object Shape", NULL)) {
            ImGui::Text("Choose Shape:");

            // Button for Cube
            if (ImGui::Button("Cube")) {
                scene.addObject(new Object("Cube", createCubeVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();  // Close the popup after adding the object
            }

            // Button for Triangle
            if (ImGui::Button("Triangle")) {
                scene.addObject(new Object("Triangle", createTriangle3DVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();  // Close the popup after adding the object
            }

            // Button for Sphere
            if (ImGui::Button("Sphere")) {
                scene.addObject(new Object("Sphere", createSphereVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();  // Close the popup after adding the object
            }

            // "Cancel" Button
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();  // Close the popup if the cancel button is pressed
            }

            ImGui::EndPopup();
        }
    }
}

void Interface::displayObjectProperties(Object* object, int index) {
    ImGui::PushID(index);

    /*ImGui::PushFont(headingFont);
    ImGui::Text("General");
    ImGui::PopFont();

    static char tempName[128] = {};
    static bool nameInitialized = false;
    if (!nameInitialized) {
        strncpy_s(tempName, object->name.c_str(), sizeof(tempName) - 1);
        nameInitialized = true;
    }

    ImGui::InputText("Object Name##", tempName, IM_ARRAYSIZE(tempName));

    if (ImGui::Button("Apply##"))
        object->setName(std::string(tempName));

    ImGui::Separator();*/

    ImGui::PushFont(headingFont);
    ImGui::Text("Transform");
    ImGui::PopFont();

    // Display Position, Rotation, and Scale
    ImGui::DragFloat3("Position", glm::value_ptr(object->position), 0.1f);
    ImGui::DragFloat3("Rotation", glm::value_ptr(object->rotation), 1.0f, -180.0f, 180.0f);
    ImGui::DragFloat3("Scale", glm::value_ptr(object->scale), 0.01f, 0.1f, 10.0f);

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("Texture/Color");
    ImGui::PopFont();

    // Show current texture or message
    if (object->texture) {
        ImGui::Text("Current Texture: %s", object->texture->getFilePath().c_str());
    }
    else {
        ImGui::Text("No texture applied");
    }

    if (ImGui::Button("Choose Texture")) {
        IGFD::FileDialogConfig config;
        config.path = "assets/";
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseTex",
            "Select Texture",
            "All Supported{.png,.jpg,.jpeg,.bmp},.png,.jpg,.jpeg,.bmp",
            config
        );
        textureTargetObject = object;
    }

    ImGui::SameLine();

    // Add button to remove texture and apply color
    if (ImGui::Button("Remove Texture")) {
        object->removeTexture();  // Remove texture
        object->setColor(glm::vec3(1.0f, 1.0f, 1.0f));  // Example: Apply white color if texture is removed
    }

    if (!object->texture) {  // Only show color controls if there's no texture
        static glm::vec3 color = object->color;
        ImGui::ColorEdit3("Object Color", &color[0]);
        if (ImGui::Button("Apply Color")) {
            object->setColor(color);  // Apply color if no texture
        }
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseTex")) {
        if (ImGuiFileDialog::Instance()->IsOk() && textureTargetObject) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            textureTargetObject->setTexture(new Texture(filePathName));  // Apply to correct object
        }
        ImGuiFileDialog::Instance()->Close();
        textureTargetObject = nullptr;  // Reset
    }



    ImGui::PopID();
}

