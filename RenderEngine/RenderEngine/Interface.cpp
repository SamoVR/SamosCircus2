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

    // Get the monitor's DPI scaling factor
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    // Adjust font scaling based on DPI scale (average scale factor for both axes)
    io.FontGlobalScale = (xscale + yscale) / 2.0f;

    // Load font
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

    sceneControlsUI();
    objectListUI();
    settingsUI();
    updateFileBrowsers();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::updateFileBrowsers()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseTex")) {
        if (ImGuiFileDialog::Instance()->IsOk() && textureTargetObject) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            textureTargetObject->setTexture(new Texture(filePathName));
        }
        ImGuiFileDialog::Instance()->Close();
        textureTargetObject = nullptr;
    }
    
    if (ImGuiFileDialog::Instance()->Display("SaveSceneDialog")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene.saveToFile(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("LoadSceneDialog")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene.loadFromFile(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

}

void Interface::settingsUI() {

    ImGui::Begin("Settings");

    ImGui::PushFont(headingFont);
    ImGui::Text("Camera");
    ImGui::PopFont();

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("UI");
    ImGui::PopFont();

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

    // Save Scene Button with file dialog
    if (ImGui::Button("Save Scene")) {
        openSaveDialog();  // Open the file save dialog
    }

    ImGui::SameLine();

    // Load Scene Button with file dialog
    if (ImGui::Button("Load Scene")) {
        openLoadDialog();  // Open the file load dialog
    }

    ImGui::End();
}

void Interface::openSaveDialog() {
    IGFD::FileDialogConfig config;
    config.path = "assets/scenes";

    ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog", "Save Scene", ".json", config);
}

void Interface::openLoadDialog() {
    IGFD::FileDialogConfig config;
    config.path = "assets/scenes";

    ImGuiFileDialog::Instance()->OpenDialog("LoadSceneDialog", "Load Scene", ".json", config);
}


void Interface::showShapeSelectionPopup() {
    if (showShapePopup) {
        if (ImGui::BeginPopupModal("Select Object Shape", NULL)) {
            ImGui::Text("Choose Shape:");

            if (ImGui::Button("Cube")) {
                scene.addObject(new Object("Cube", createCubeVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Triangle")) {
                scene.addObject(new Object("Triangle", createTriangle3DVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("Sphere")) {
                scene.addObject(new Object("Sphere", createSphereVertices(), defaultTexture));
                ImGui::CloseCurrentPopup();
            }

            ImGui::Separator();

            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup(); 
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

    if (ImGui::Button("Remove Texture")) {
        object->removeTexture();
        object->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    }

    if (!object->texture) {
        static glm::vec3 color = object->color;
        ImGui::ColorEdit3("Object Color", &color[0]);
        if (ImGui::Button("Apply Color")) {
            object->setColor(color);
        }
    }

    ImGui::PopID();
}

